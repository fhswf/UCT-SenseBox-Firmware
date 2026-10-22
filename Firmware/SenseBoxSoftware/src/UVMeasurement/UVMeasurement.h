#pragma once

#include "../SensorMeasurement/SensorMeasurement.h"

#ifndef VEML_INCLUDED
#define VEML_INCLUDED
#include <VEML6070.h>
#endif

class UVMeasurement:public SensorMeasurement<float,600>
{
    VEML6070 VEML_Sensor;
public:
    UVMeasurement(String name_sensor, int interval_ms);
    void add_to_lora_message(LoraMessage *msg);
    void initialize();
protected:
    void perform_measurement(MeasurementValue<float> *val_buffer);
};