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
    void update();

    const Scd41State& getState();

private:
    SensirionI2cScd4x sensor;
    Scd41State state;

    bool started = false;
    uint32_t lastCheckMs = 0;

    static constexpr uint8_t I2C_ADDRESS = 0x62;
    static constexpr uint32_t CHECK_INTERVAL_MS = 500;
};