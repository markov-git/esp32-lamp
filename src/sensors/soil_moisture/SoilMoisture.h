#pragma once

#include <Arduino.h>

struct SoilMoistureState
{
    uint16_t raw;
};

class SoilMoisture
{
public:
    bool begin();
    SoilMoistureState getState() const;

private:
    static constexpr uint8_t SENSOR_PIN = 34;
};