/*
MessageSender.h

This class implements the wrapper to the LoRa LMIC interface. The event methods
("notify_...()") drive the LoRa status state machine and are being called from  
the onEvent() function in the main program (which is, again, automatically called
by the LMIC library).
*/

#pragma once

#include "../../global_defines.h"

#include <Arduino.h>
#include "lmic.h"
#include <LoraMessage.h>

#include "../Task/Task.h"
#include "../SensorMeasurement/SensorMeasurement.h"
#include "../WatchDog/WatchDog.h"

class MessageSender: public Task
{
    const uint16_t FRAME_FORMAT_VERSION = 1;
    
    const uint8_t STATUS_DISCONNECTED = 0;
    const uint8_t STATUS_CONNECTING = 1;
    const uint8_t STATUS_CONNECTED = 2;
    const uint8_t STATUS_SENDING = 3;

    uint16_t lora_sending_interval_sec = LORA_SENDING_INTERVAL_S;
    uint32_t lora_status_interval_sec = LORA_STATUS_SENDING_INTERVAL_S;
    uint16_t lora_max_time_joining_sec = LORA_JOIN_TIMEOUT_TIME_S;
    uint16_t lora_max_time_tx_sec = LORA_SENDING_TIMEOUT_TIME_S;
    
    uint8_t status = STATUS_DISCONNECTED;
    uint64_t time_sending_started = 0;
    uint64_t time_next_measurement_frame = 0;
    uint64_t time_next_status_frame = 0;
    uint8_t join_number_trys = 0;

    Task* tasks_to_send[NUMBER_TASKS];
    WatchDog* watchdog;
    unsigned int number_tasks_to_send = 0;

public:
    void run();
    void connect();
    void append_task(Task* t);     // is called to append a task to the tasks_to_send list, so that it is considered when sending a measurement frame.
    void notify_joined();
    void notify_sent();
    void notify_join_failed();
    void notify_cyclical_log_save();
    void initialize();              // is called after the initialization of timer and logger, in oder to adjust sending times to the correct time base
    void set_watchdog(WatchDog* wd);
    void computeLoRaCredentials();
    MessageSender();
private:    
    int check_lora_status();
    int send(LoraMessage *m, uint8_t fport=LORA_FPORT_STATUS);
    void prepare_status_message(LoraMessage *m);
    void prepare_measurement_message(LoraMessage *m);
};