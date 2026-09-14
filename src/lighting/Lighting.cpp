#include "Lighting.h"

#include <Arduino.h>

namespace
{
    constexpr uint8_t PWM_FREQUENCY = 1000;
    constexpr uint8_t PWM_RESOLUTION = 8;

    // Пока подключен только тестовый LED.
    constexpr uint8_t TEST_PIN = 16;
    constexpr uint8_t TEST_PWM_CHANNEL = 0;
}

void Lighting::begin()
{
    ledcSetup(
        TEST_PWM_CHANNEL,
        PWM_FREQUENCY,
        PWM_RESOLUTION
    );

    ledcAttachPin(
        TEST_PIN,
        TEST_PWM_CHANNEL
    );

    // Все каналы начинаются выключенными.
    for (uint8_t lamp = 0; lamp < LAMP_COUNT; lamp++)
    {
        for (uint8_t channel = 0; channel < CHANNELS_PER_LAMP; channel++)
        {
            brightness[lamp][channel] = 0;
        }
    }
}

void Lighting::setBrightness(
    Lamp lamp,
    Channel channel,
    uint8_t percent
)
{
    percent = constrain(percent, 0, 100);

    const uint8_t lampIndex = getLampIndex(lamp);
    const uint8_t channelIndex = getChannelIndex(channel);

    brightness[lampIndex][channelIndex] = percent;

    // Пока физически существует только Lamp1 / Red.
    if (
        lamp == Lamp::Lamp1 &&
        channel == Channel::Red
    )
    {
        const uint32_t duty =
            map(percent, 0, 100, 0, 255);

        ledcWrite(
            TEST_PWM_CHANNEL,
            duty
        );
    }
}

uint8_t Lighting::getBrightness(
    Lamp lamp,
    Channel channel
) const
{
    return brightness[
        getLampIndex(lamp)
    ][
        getChannelIndex(channel)
    ];
}

uint8_t Lighting::getLampIndex(Lamp lamp) const
{
    return static_cast<uint8_t>(lamp);
}

uint8_t Lighting::getChannelIndex(Channel channel) const
{
    return static_cast<uint8_t>(channel);
}