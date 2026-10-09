#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>

struct Scd41State {
    uint16_t co2Ppm = 0;
    float temperature = NAN;
    float humidity = NAN;
    bool available = false;
};

class Scd41 {
public:
    bool begin();

    Scd41State getState();

private:
    SensirionI2cScd4x sensor;
    Scd41State state;

    bool initialized = false;

    static constexpr uint8_t I2C_ADDRESS = 0x62;
};