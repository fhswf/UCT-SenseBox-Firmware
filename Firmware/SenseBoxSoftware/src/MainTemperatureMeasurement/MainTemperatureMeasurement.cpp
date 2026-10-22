#include "MainTemperatureMeasurement.h"

MainTemperatureMeasurement::MainTemperatureMeasurement(String name_sensor, int interval_ms, Adafruit_HDC1000 *HDC_sensor):SensorMeasurement<float,600>(name_sensor, interval_ms)
{
    this->HDC_sensor = HDC_sensor;      // HDC sensor needs to be initialized in setup() function by calling HDC.begin()!
}

void MainTemperatureMeasurement::add_to_lora_message(LoraMessage *msg)
{
    msg->addUint16((uint16_t)((get_mean()+40)*595));
    msg->addUint16((uint16_t)((get_min()+40)*595));
    msg->addUint16((uint16_t)((get_max()+40)*595));
}

void MainTemperatureMeasurement::perform_measurement(MeasurementValue<float> *val_buffer)
{
    float meas_value = HDC_sensor->readTemperature();
    MeasurementValidity validity;
    if ((meas_value != 0.000) && (meas_value > -30.0) && !(meas_value > 42.490 && meas_value < 42.505) && !isnan(meas_value))
        validity = valid;
    else validity = invalid;

    val_buffer->set_value(meas_value,validity);
}
