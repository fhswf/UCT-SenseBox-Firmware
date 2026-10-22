#pragma once

#include "../SensorMeasurement/SensorMeasurement.h"
#include <LTR329.h>

typedef enum {
    TSL = 0,
    LTR = 1
} LightSensorType;

class LuxMeasurement:public SensorMeasurement<uint32_t,600>
{
    LTR329 ltr_sensor;
    LightSensorType sensor_type = TSL;
    const byte integration_time = 0, measurement_rate = 3, gain = 1;
public:
    LuxMeasurement(String name_sensor, int interval_ms);
    void add_to_lora_message(LoraMessage *msg);
    void initialize();
protected:
    void perform_measurement(MeasurementValue<uint32_t> *val_buffer);
    int read_register(byte address, byte reg);
    void write_register(byte address, byte reg, byte value);
    uint32_t get_measurement_value();
};