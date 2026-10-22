#pragma once

#include "../SensorMeasurement/SensorMeasurement.h"
#include "../AirPressureSensor/AirPressureSensor.h"

class BoxTemperatureMeasurement:public SensorMeasurement<float,60>
{
    AirPressureSensor *sensor;
public:
    BoxTemperatureMeasurement(String name_sensor, int interval_ms, AirPressureSensor *sensor);
    void add_to_lora_message(LoraMessage *msg);
protected:
    void perform_measurement(MeasurementValue<float> *val_buffer);
};