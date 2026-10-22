#include "UVMeasurement.h"

UVMeasurement::UVMeasurement(String name_sensor, int interval_ms):SensorMeasurement<float,600>(name_sensor, interval_ms)
{
    
}

void UVMeasurement::initialize()
{
    VEML_Sensor.begin();
}

void UVMeasurement::perform_measurement(MeasurementValue<float> *val_buffer)
{
    float meas_value = VEML_Sensor.getUV();
    MeasurementValidity validity = valid;           // ToDo
    if (isnan(meas_value)) validity = invalid;

    val_buffer->set_value(meas_value,validity);
}


void UVMeasurement::add_to_lora_message(LoraMessage *msg)
{
    uint32_t buf = get_mean();
    msg->addUint8(buf % 256);   // das niederwertigste Byte
    msg->addUint16((uint32_t)buf >> 8);   // die zwei höherwertigen Byte des 24-Bit-Werts in Big Endian

    buf = get_min();
    msg->addUint8(buf % 256);   // das niederwertigste Byte
    msg->addUint16((uint32_t)buf >> 8);   // die zwei höherwertigen Byte des 24-Bit-Werts in Big Endian

    buf = get_max();
    msg->addUint8(buf % 256);   // das niederwertigste Byte
    msg->addUint16((uint32_t)buf >> 8);   // die zwei höherwertigen Byte des 24-Bit-Werts in Big Endian
}
