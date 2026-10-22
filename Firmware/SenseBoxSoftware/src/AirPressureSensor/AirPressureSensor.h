#pragma once

#include <Adafruit_BMP280.h>
#include <Adafruit_DPS310.h>

#include "../SensorMeasurement/MeasurementValue.h"

class AirPressureSensor
{
    Adafruit_BMP280 BMP_Sensor = Adafruit_BMP280();
    Adafruit_DPS310 DPS_Sensor = Adafruit_DPS310();
    enum {bmp, dps} sensortyp;

public:
    void initialize(); 
    void get_pressure_measurement(MeasurementValue<float> *value);   
    void get_temperature_measurement(MeasurementValue<float> *value);
};