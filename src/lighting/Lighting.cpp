#include "Lighting.h"

namespace
{
    constexpr uint8_t MAX_BRIGHTNESS = 100;
}

void Lighting::begin()
{
    manualState = {};
    currentState = {};

    ledcSetup(
        TEST_PWM_CHANNEL,
        PWM_FREQUENCY,
        PWM_RESOLUTION
    );

    ledcAttachPin(
        TEST_PIN,
        TEST_PWM_CHANNEL
    );

    applyBrightness(
        Lamp::Lamp1,
        Channel::Red,
        0
    );
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
    // Currently only the physical test channel exists.
    if (
        lamp == Lamp::Lamp1 &&
        channel == Channel::Red
    )
    {
        const uint8_t duty =
            static_cast<uint16_t>(percent) *
            255 /
            100;

        ledcWrite(
            TEST_PWM_CHANNEL,
            duty
        );
    }
}