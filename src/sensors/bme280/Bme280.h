#pragma once

#include <Arduino.h>
#include <Adafruit_BME280.h>

struct Bme280State
{
    float temperature;
    float humidity;
    float pressure;
};

class Bme280
{
public:
    bool begin();

    Bme280State getState();

private:
    static constexpr uint8_t SDA_PIN = 21;
    static constexpr uint8_t SCL_PIN = 22;

    Adafruit_BME280 sensor;
};