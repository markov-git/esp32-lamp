#pragma once

#include <cstdint>
#include <Arduino.h>

enum class Lamp : uint8_t
{
    Lamp1 = 0,
    Lamp2 = 1,
    Lamp3 = 2
};

enum class Channel : uint8_t
{
    Red = 0,
    Blue = 1
};

struct LampState
{
    uint8_t red;
    uint8_t blue;
};

struct LightingState
{
    LampState lamps[3];

    bool operator==(
        const LightingState& other
    ) const
    {
        for (uint8_t i = 0; i < 3; i++)
        {
            if (
                lamps[i].red != other.lamps[i].red ||
                lamps[i].blue != other.lamps[i].blue
            )
            {
                return false;
            }
        }

        return true;
    }

    bool operator!=(
        const LightingState& other
    ) const
    {
        return !(*this == other);
    }
};

class Lighting
{
public:
    static constexpr uint8_t LAMP_COUNT = 3;

    void begin();

    bool setManualBrightness(
        Lamp lamp,
        Channel channel,
        uint8_t percent
    );

    uint8_t getManualBrightness(
        Lamp lamp,
        Channel channel
    ) const;

    LightingState getManualState() const;

    void setState(
        const LightingState& state
    );

    LightingState getState() const;

private:
    static constexpr uint8_t CHANNELS_PER_LAMP = 2;

    static constexpr uint32_t PWM_FREQUENCY = 1000;
    static constexpr uint8_t PWM_RESOLUTION = 8;

    static constexpr uint8_t TEST_PIN = 16;
    static constexpr uint8_t TEST_PWM_CHANNEL = 0;

    LightingState manualState{};
    LightingState currentState{};

    uint8_t getLampIndex(Lamp lamp) const;
    uint8_t getChannelIndex(Channel channel) const;

    void applyBrightness(
        Lamp lamp,
        Channel channel,
        uint8_t percent
    );
};