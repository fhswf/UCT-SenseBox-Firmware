#include "BoxTemperatureMeasurement.h"

BoxTemperatureMeasurement::BoxTemperatureMeasurement(String name_sensor, int interval_ms, AirPressureSensor *sensor):SensorMeasurement<float,60>(name_sensor, interval_ms)
{
    this->sensor = sensor;      // BMP sensor needs to be initialized in setup() function by calling BMP.begin()!
}

void BoxTemperatureMeasurement::add_to_lora_message(LoraMessage *msg)
{
    float mean = get_mean();
    msg->addUint8((uint8_t)(mean+40)*2);
}

void BoxTemperatureMeasurement::perform_measurement(MeasurementValue<float> *val_buffer)
{
    sensor->get_temperature_measurement(val_buffer);
}
