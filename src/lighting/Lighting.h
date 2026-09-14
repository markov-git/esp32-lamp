#pragma once

#include <cstdint>

class Lighting
{
public:
    void begin();

    void setBrightness(uint8_t percent);
    uint8_t getBrightness() const;

private:
    static constexpr uint8_t LED_PIN = 16;

    static constexpr uint8_t PWM_CHANNEL = 0;
    static constexpr uint32_t PWM_FREQUENCY = 1000;
    static constexpr uint8_t PWM_RESOLUTION = 8;

    uint8_t brightness = 0;
};