#include "Bme280.h"

#include <Wire.h>

bool Bme280::begin()
{
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!sensor.begin(0x76))
    {
        return false;
    }

    sensor.setSampling(
        Adafruit_BME280::MODE_NORMAL,
        Adafruit_BME280::SAMPLING_X2, // temperature
        Adafruit_BME280::SAMPLING_X2, // pressure
        Adafruit_BME280::SAMPLING_X2, // humidity
        Adafruit_BME280::FILTER_X4,
        Adafruit_BME280::STANDBY_MS_1000
    );

    return true;
}

Bme280State Bme280::getState()
{
    Bme280State state{};

    state.temperature = sensor.readTemperature();
    state.humidity = sensor.readHumidity();
    state.pressure = sensor.readPressure() / 100.0F;

    return state;
}