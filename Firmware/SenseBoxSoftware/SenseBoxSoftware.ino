/*
SenseBoxSoftware.ino
The main program code.
*/


#include "global_defines.h"

#include <LoraMessage.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <senseBoxIO.h>
#include "src/SenseBoxConfig/sensebox_config.h"

#include "src/Task/Task.h"
#include "src/SensorMeasurement/SensorMeasurement.h"
#include "src/MainTemperatureMeasurement/MainTemperatureMeasurement.h"
#include "src/HumidityMeasurement/HumidityMeasurement.h"
#include "src/PressureMeasurement/PressureMeasurement.h"
#include "src/BoxTemperatureMeasurement/BoxTemperatureMeasurement.h"
#include "src/LuxMeasurement/LuxMeasurement.h"
#include "src/UVMeasurement/UVMeasurement.h"

#include "src/MessageSender/MessageSender.h"
#include "src/WatchDog/WatchDog.h"
#include "src/Timer/Timer.h"
#include "src/Logger/Logger.h"

Adafruit_HDC1000 HDC_Sensor = Adafruit_HDC1000();
AirPressureSensor pressure_temp_sensor = AirPressureSensor();
//GPS g;

Timer timer;      // Order of first timer and logger instanciation is important; also, each Task class (except Logger itself) needs to be instanciated _after_ the first Timer and Logger objects!
Logger logger;    // ... this is due to the initialization of the singleton objects, which will be referenced from Tasks.

MainTemperatureMeasurement main_temp_measurement = MainTemperatureMeasurement("main_temperature", 1*MS_PER_SEC, &HDC_Sensor);
HumidityMeasurement humidity_measurement = HumidityMeasurement("humidity", 10*MS_PER_SEC, &HDC_Sensor);
PressureMeasurement pressure_measurement = PressureMeasurement("air_pressure", 10*MS_PER_SEC, &pressure_temp_sensor);
BoxTemperatureMeasurement box_temperature_measurement = BoxTemperatureMeasurement("box_temperature", 10*MS_PER_SEC, &pressure_temp_sensor);
LuxMeasurement lux_measurement = LuxMeasurement("lux",1*MS_PER_SEC);
UVMeasurement uv_measurement = UVMeasurement("uv",1*MS_PER_SEC);

MessageSender msg_sender;
WatchDog watchdog;

// List of all task objects that need to be run (all task classes need to be derived from class Task):
// timer needs to be the first task to run, in order to update the current time for the other tasks!
Task* tasks[NUMBER_TASKS] = {&main_temp_measurement, &humidity_measurement, &pressure_measurement, &box_temperature_measurement, &lux_measurement, &uv_measurement, &msg_sender, &logger};

void setup(){    
    #if DEBUG_LEVEL > 0
    Serial.begin(115200);
    delay(10000);            // Time to start the serial monitor on a connected PC
    #endif
    
    DEBUG_HIGH_PRIO("Booting up senseBox with software "); DEBUGLN_HIGH_PRIO(FIRMWARE_VERSION);

    Logger::get_logger()->initialize();   // Initialize Logger by reading back the latest logging state from flash memory
                                          // This will also initialize the timer with the last logging time from flash.
                                     
    DEBUGLN("Init Tasks...");
    watchdog.set_supervised_tasks(&tasks);

    msg_sender.initialize();              // initialize Message Sender sending times
    msg_sender.set_watchdog(&watchdog);

    // Append the tasks that send data in a measurement frame to the MessageSender object's list of task, in the correct order of occurrence in the message frame:
    msg_sender.append_task(&main_temp_measurement);
    msg_sender.append_task(&humidity_measurement);
    msg_sender.append_task(&pressure_measurement);
    msg_sender.append_task(&lux_measurement);
    msg_sender.append_task(&uv_measurement);
    msg_sender.append_task(&box_temperature_measurement);

    msg_sender.computeLoRaCredentials();  // Compute Device EUI and App-Key from Microcontroller Serial Number

    DEBUGLN("Init Devices...");
    senseBoxIO.powerXB1(false); // power off to reset RFM9X
    delay(250);   // Original SenseBox SW: 250 ms
    senseBoxIO.powerXB1(true);  // power on
    delay(250);   // Original SenseBox SW: 250 ms
    senseBoxIO.powerUART(true);

    // The original senseBox software has some 3s delays between the following initialization steps; these have been left out here.
    HDC_Sensor.begin();
    pressure_temp_sensor.initialize();
    lux_measurement.initialize();
    uv_measurement.initialize();

    DEBUGLN("Init LoRaWAN...");
    os_init();

    DEBUGLN("Start WatchDog...");
    watchdog.setup(WD_TOLERANCE_TIME_MS, &timer);

    #if DEBUG_LEVEL > 0    
    delay(2000);            // Time to read messages in the serial monitor on a connected PC
    #endif
}

void loop(){    
    os_runloop_once();
    for (int i=0; i<NUMBER_TASKS; i++){
        tasks[i]->check_and_perform_next_run();        
    }
    delay(1);
    watchdog.notify_milestone(WD_MILESTONE_EXIT_LOOP);
}


// LoRaWAN event handler. Will be automatically called by the LoRaWAN framework!
void onEvent (ev_t ev) {
    DEBUG(timer.get_current_time_ms());
    DEBUG(": LoRaWAN-Event: ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
          DEBUGLN(F("EV_SCAN_TIMEOUT"));
          break;
        case EV_BEACON_FOUND:
          DEBUGLN(F("EV_BEACON_FOUND"));
          break;
        case EV_BEACON_MISSED:
          DEBUGLN(F("EV_BEACON_MISSED"));
          break;
        case EV_BEACON_TRACKED:
          DEBUGLN(F("EV_BEACON_TRACKED"));
          break;
        case EV_JOINING:                          // Verbindungsaufbau zum LoRaWAN
          DEBUGLN(F("EV_JOINING"));
          break;
        case EV_JOINED:                           // Verbindungsaufbau abgeschlossen
          DEBUGLN(F("EV_JOINED"));
          msg_sender.notify_joined();
          break;
        case EV_RFU1:
          DEBUGLN(F("EV_RFU1"));
          break;
        case EV_JOIN_FAILED:
          DEBUGLN(F("EV_JOIN_FAILED"));
          msg_sender.notify_join_failed();
          break;
        case EV_REJOIN_FAILED:
          DEBUGLN(F("EV_REJOIN_FAILED"));
          msg_sender.notify_join_failed();
          break;
        case EV_TXCOMPLETE:                     // Senden abgeschlossen
          DEBUGLN(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
          if (LMIC.txrxFlags & TXRX_ACK)
            DEBUGLN(F("Received ack"));
          if (LMIC.dataLen) {
            DEBUG(F("Received "));
            DEBUG(LMIC.dataLen);
            DEBUGLN(F(" bytes of payload"));
          }
          msg_sender.notify_sent();
          break;
        case EV_LOST_TSYNC:
          DEBUGLN(F("EV_LOST_TSYNC"));
          break;
        case EV_RESET:
          DEBUGLN(F("EV_RESET"));
          break;
        case EV_RXCOMPLETE:
          // data received in ping slot
          DEBUGLN(F("EV_RXCOMPLETE"));
          break;
        case EV_LINK_DEAD:
          DEBUGLN(F("EV_LINK_DEAD"));
          break;
        case EV_LINK_ALIVE:
          DEBUGLN(F("EV_LINK_ALIVE"));
          break;
        default:
          DEBUGLN(F("Unknown event"));
          break;
      }
}