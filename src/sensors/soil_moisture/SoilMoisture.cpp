#include "SoilMoisture.h"

namespace
{
    constexpr uint8_t SENSOR_PINS[SoilMoisture::SENSOR_COUNT] = {
        34,
        35,
        32
    };

    struct Calibration
    {
        uint16_t dry;
        uint16_t wet;
    };

    constexpr Calibration CALIBRATION[SoilMoisture::SENSOR_COUNT] = {
        {2464, 2170}, // Sensor 1
        {2443, 1961}, // Sensor 2
        {2550, 2280}  // Sensor 3
    };
}

bool SoilMoisture::begin()
{
    for (uint8_t i = 0; i < SENSOR_COUNT; i++)
    {
        pinMode(SENSOR_PINS[i], INPUT);
    }

    analogReadResolution(12);

    return true;
}

SoilMoistureState SoilMoisture::getState() const
{
    SoilMoistureState state{};

    for (uint8_t i = 0; i < SENSOR_COUNT; i++)
    {
        state.raw[i] = analogRead(SENSOR_PINS[i]);

        state.percent[i] = calculatePercent(
            state.raw[i],
            CALIBRATION[i].dry,
            CALIBRATION[i].wet
        );
    }

    return state;
}

uint8_t SoilMoisture::calculatePercent(
    uint16_t raw,
    uint16_t dry,
    uint16_t wet
) const
{
    if (raw >= dry)
    {
        return 0;
    }

    if (raw <= wet)
    {
        return 100;
    }

    const float percent =
        static_cast<float>(dry - raw) /
        static_cast<float>(dry - wet) *
        100.0F;

    return static_cast<uint8_t>(percent);
}