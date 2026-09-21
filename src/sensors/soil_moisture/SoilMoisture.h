#pragma once

#include <Arduino.h>

struct SoilMoistureState
{
    uint16_t raw[3];
    uint8_t percent[3];
};

class SoilMoisture
{
public:
    static constexpr uint8_t SENSOR_COUNT = 3;

    bool begin();
    SoilMoistureState getState() const;

private:
    uint8_t calculatePercent(
        uint16_t raw,
        uint16_t dry,
        uint16_t wet
    ) const;
};