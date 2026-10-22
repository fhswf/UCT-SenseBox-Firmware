#include <Arduino.h>
#include "AirPressureSensor.h"
#include "../Timer/Timer.h"
#include "../../global_defines.h"



void AirPressureSensor::initialize(){
    // This method may earliest be called in setup() function
    // To test whether box is equipped with bmp or dps sensor, initiallize dps and test whether it is then ready to perform measurements:
    DPS_Sensor.begin_I2C(0x76);
    DPS_Sensor.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    DPS_Sensor.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
    Timer *t = Timer::get_timer();

    if(DPS_Sensor.temperatureAvailable() && DPS_Sensor.pressureAvailable()){
        this->sensortyp = dps;
        DEBUG_HIGH_PRIO(t->get_current_time_ms()); DEBUGLN_HIGH_PRIO(": DPS310 air pressure sensor detected.");
    }
    else    // No DPS sensor => initialize bmp
    {
        this->sensortyp = bmp;
        BMP_Sensor.begin(0x76);
        DEBUG_HIGH_PRIO(t->get_current_time_ms()); DEBUGLN_HIGH_PRIO(": BMP280 air pressure sensor detected.");
    }    
}


void AirPressureSensor::get_pressure_measurement(MeasurementValue<float> *value)
{
    float meas_value = 0;
    if (this->sensortyp==bmp){
        meas_value = BMP_Sensor.readPressure()/100;
    }
    else{
        sensors_event_t pressure_event;
        DPS_Sensor.getEvents(NULL, &pressure_event);
        meas_value = pressure_event.pressure;
    }
    
    MeasurementValidity validity = valid;
    if (isnan(meas_value)) validity = invalid;

    value->set_value(meas_value,validity);
}


void AirPressureSensor::get_temperature_measurement(MeasurementValue<float> *value)
{
    float meas_value = 0;
    if (this->sensortyp==bmp){
        meas_value = BMP_Sensor.readTemperature();
    }
    else{
        sensors_event_t temperature_event;
        DPS_Sensor.getEvents(&temperature_event, NULL);
        meas_value = temperature_event.temperature;
    }
    
    MeasurementValidity validity = valid;
    if (isnan(meas_value)) validity = invalid;

    value->set_value(meas_value,validity);
}