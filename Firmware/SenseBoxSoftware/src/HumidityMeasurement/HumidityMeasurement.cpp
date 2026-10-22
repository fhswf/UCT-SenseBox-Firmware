#include "HumidityMeasurement.h"

HumidityMeasurement::HumidityMeasurement(String name_sensor, int interval_ms, Adafruit_HDC1000 *HDC_sensor):SensorMeasurement<float,60>(name_sensor, interval_ms)
{
    this->HDC_sensor = HDC_sensor;      // HDC sensor needs to be initialized in setup() function by calling HDC.begin()!
}

void HumidityMeasurement::add_to_lora_message(LoraMessage *msg)
{
    int16_t mean = get_mean() * 100;
    msg->addUint16(mean);
}

void HumidityMeasurement::perform_measurement(MeasurementValue<float> *val_buffer)
{
    float meas_value = HDC_sensor->readHumidity();
    MeasurementValidity validity = valid;
    if (isnan(meas_value)) validity = invalid;

    val_buffer->set_value(meas_value,validity);
}
