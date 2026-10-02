#include "Lighting.h"

namespace
{
    constexpr uint8_t MAX_BRIGHTNESS = 100;
}

void Lighting::begin()
{
    manualState = {};
    currentState = {};

    for (uint8_t i = 0; i < PWM_PIN_COUNT; i++)
    {
        ledcSetup(
            PWM_CHANNELS[i],
            PWM_FREQUENCY,
            PWM_RESOLUTION
        );

        ledcAttachPin(
            PWM_PINS[i],
            PWM_CHANNELS[i]
        );

        ledcWrite(
            PWM_CHANNELS[i],
            0
        );
    }
}

bool Lighting::setManualBrightness(
    Lamp lamp,
    Channel channel,
    uint8_t percent
)
{
    if (percent > MAX_BRIGHTNESS)
    {
        return false;
    }

    const uint8_t lampIndex =
        getLampIndex(lamp);

    if (channel == Channel::Red)
    {
        manualState.lamps[lampIndex].red =
            percent;
    }
    else
    {
        manualState.lamps[lampIndex].blue =
            percent;
    }

    return true;
}

uint8_t Lighting::getManualBrightness(
    Lamp lamp,
    Channel channel
) const
{
    const uint8_t lampIndex =
        getLampIndex(lamp);

    if (channel == Channel::Red)
    {
        return manualState.lamps[lampIndex].red;
    }

    return manualState.lamps[lampIndex].blue;
}

LightingState Lighting::getManualState() const
{
    return manualState;
}

void Lighting::setState(
    const LightingState& state
)
{
    for (uint8_t lampIndex = 0;
        lampIndex < LAMP_COUNT;
        lampIndex++)
    {
        const Lamp lamp =
            static_cast<Lamp>(lampIndex);

        const LampState& current =
            currentState.lamps[lampIndex];

        const LampState& next =
            state.lamps[lampIndex];

        if (current.red != next.red)
        {
            applyBrightness(
                lamp,
                Channel::Red,
                next.red
            );
        }

        if (current.blue != next.blue)
        {
            applyBrightness(
                lamp,
                Channel::Blue,
                next.blue
            );
        }
    }

    currentState = state;
}

LightingState Lighting::getState() const
{
    return currentState;
}

uint8_t Lighting::getLampIndex(Lamp lamp) const
{
    return static_cast<uint8_t>(lamp);
}

uint8_t Lighting::getPwmIndex(
    Lamp lamp,
    Channel channel
) const
{
    return getLampIndex(lamp) * CHANNELS_PER_LAMP +
        static_cast<uint8_t>(channel);
}

uint8_t Lighting::getChannelIndex(Channel channel) const
{
    return static_cast<uint8_t>(channel);
}

void Lighting::applyBrightness(
    Lamp lamp,
    Channel channel,
    uint8_t percent
)
{
    const uint8_t index =
        getPwmIndex(lamp, channel);

    const uint8_t pwmValue =
        static_cast<uint16_t>(percent) * 255 / 100;

    ledcWrite(
        PWM_CHANNELS[index],
        pwmValue
    );
}