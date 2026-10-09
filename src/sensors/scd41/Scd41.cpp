#include "Scd41.h"

bool Scd41::begin()
{
    sensor.begin(Wire, I2C_ADDRESS);

    const uint16_t error =
        sensor.startPeriodicMeasurement();

    if (error != 0)
    {
        Serial.print("SCD41: start failed: ");
        Serial.println(error);

        initialized = false;
        return false;
    }

    initialized = true;

    Serial.println("SCD41: periodic measurement started");

    return true;
}

Scd41State Scd41::getState()
{
    if (!initialized)
        return state;

    bool dataReady = false;

    uint16_t error =
        sensor.getDataReadyStatus(dataReady);

    if (error != 0)
    {
        Serial.print("SCD41: data-ready check failed: ");
        Serial.println(error);

        return state;
    }

    if (!dataReady)
        return state;

    uint16_t co2 = 0;
    float temperature = NAN;
    float humidity = NAN;

    error = sensor.readMeasurement(
        co2,
        temperature,
        humidity
    );

    if (error != 0)
    {
        Serial.print("SCD41: read failed: ");
        Serial.println(error);

        return state;
    }

    if (co2 == 0 ||
        !isfinite(temperature) ||
        !isfinite(humidity) ||
        humidity < 0.0f ||
        humidity > 100.0f)
    {
        Serial.println("SCD41: invalid measurement");
        return state;
    }

    state.co2Ppm = co2;
    state.temperature = temperature;
    state.humidity = humidity;
    state.available = true;

    return state;
}