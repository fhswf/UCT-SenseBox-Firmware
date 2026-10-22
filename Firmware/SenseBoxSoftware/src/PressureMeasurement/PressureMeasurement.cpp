#include "PressureMeasurement.h"

PressureMeasurement::PressureMeasurement(String name_sensor, int interval_ms, AirPressureSensor *sensor):SensorMeasurement<float,60>(name_sensor, interval_ms)
{
    this->sensor = sensor;      // BMP sensor needs to be initialized in setup() function by calling BMP.begin()!
}

void PressureMeasurement::add_to_lora_message(LoraMessage *msg)
{
    msg->addUint16((get_mean()-300)*81.9187);
}

void PressureMeasurement::perform_measurement(MeasurementValue<float> *val_buffer)
{
    sensor->get_pressure_measurement(val_buffer);
}
