#pragma once

#include <cstdint>

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
};

class Lighting
{
public:
    static constexpr uint8_t LAMP_COUNT = 3;

    void begin();

    void setBrightness(
        Lamp lamp,
        Channel channel,
        uint8_t percent
    );

    uint8_t getBrightness(
        Lamp lamp,
        Channel channel
    ) const;

    LightingState getState() const;

private:
    static constexpr uint8_t CHANNELS_PER_LAMP = 2;

    uint8_t brightness[LAMP_COUNT][CHANNELS_PER_LAMP] = {};

    uint8_t getLampIndex(Lamp lamp) const;
    uint8_t getChannelIndex(Channel channel) const;
};