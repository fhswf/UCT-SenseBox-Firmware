#include "LuxMeasurement.h"
#include "Wire.h"            // For the arduino::Wire Functionality

LuxMeasurement::LuxMeasurement(String name_sensor, int interval_ms):SensorMeasurement<uint32_t,600>(name_sensor, interval_ms)
{
    
}

void LuxMeasurement::initialize(){
    /* Initializes I2C Communication & Lux Sensor, to be called from setup() function */
    Wire.begin();
    DEBUG(F("Checking light sensor type... "));

    byte u = read_register(0x29, 0x80 | 0x0A);      // id register
    if ((u & 0xF0) == 0xA0) {                       // TSL45315 detected
        DEBUG_HIGH_PRIO(timer->get_current_time_ms());
        DEBUGLN_HIGH_PRIO(F(": TSL45315 Lux sensor detected!"));
        sensor_type = TSL; //TSL45315
        write_register(0x29, 0x80 | 0x00, 0x03); //control: power on
        write_register(0x29, 0x80 | 0x01, 0x02); //config: M=4 T=100ms
        delay(120);        
    }
    else {
        DEBUG_HIGH_PRIO(timer->get_current_time_ms());
        DEBUGLN_HIGH_PRIO(F(": LTR329 Lux sensor detected!"));
        sensor_type = LTR;
        ltr_sensor.begin();
        ltr_sensor.setControl(gain, false, false);
        ltr_sensor.setMeasurementRate(integration_time, measurement_rate);
        ltr_sensor.setPowerUp();
        delay(10);
    }
}

void LuxMeasurement::add_to_lora_message(LoraMessage *msg)
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

void LuxMeasurement::perform_measurement(MeasurementValue<uint32_t> *val_buffer)
{
    uint32_t meas_value = get_measurement_value();
    MeasurementValidity validity = valid;    

    val_buffer->set_value(meas_value,validity);
}


uint32_t LuxMeasurement::get_measurement_value()
{
    uint lux = 0;
    if (sensor_type == TSL) {
        unsigned int u = (read_register(0x29, 0x80 | 0x04) << 0);  // read lower byte of data
        u |= (read_register(0x29, 0x80 | 0x05) << 8);              // read higher byte of data and shift accordingly
        lux = u * 4;                                    // calc lux with M=4 and T=100ms
    }
    else if (sensor_type == LTR) {
        unsigned int data0, data1;
        for (int i = 0; i < 10; i++) {  // 5 times repeated in standard senseBox software, but with 10ms waiting interval after unsuccessful read (see below)
            if (ltr_sensor.getData(data0, data1)) {
                bool result = ltr_sensor.getLux(gain, integration_time, data0, data1, lux);
                if (!result) DEBUGLN(F("LTR sensor saturated"));
                if(lux > 0) break;      // Value successfully read
                else delay(1);          // 10 ms in standard senseBox software, lower value possible / can be replaced by state machine?
            }
            else {
                DEBUG(F("LTR getData error "));
                DEBUGLN(ltr_sensor.getError());
            }
        }
    }
    return lux;
}

int LuxMeasurement::read_register(byte address, byte reg)
{
    int i = 0;

    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)address, (uint8_t)1);
    delay(1);
    if(Wire.available())
    i = Wire.read();
    return i;
}

void LuxMeasurement::write_register(byte address, byte reg, byte value)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}