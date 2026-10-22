#pragma once

#include "../SensorMeasurement/SensorMeasurement.h"

#ifndef HDC_INCLUDED
#define HDC_INCLUDED
#include <Adafruit_HDC1000.h>
#endif

class HumidityMeasurement:public SensorMeasurement<float,60>
{
    Adafruit_HDC1000 *HDC_sensor;
public:
    HumidityMeasurement(String name_sensor, int interval_ms, Adafruit_HDC1000 *HDC_sensor);
    void add_to_lora_message(LoraMessage *msg);
protected:
    void perform_measurement(MeasurementValue<float> *val_buffer);
};