#include "Lighting.h"

#include <Arduino.h>

void Lighting::begin()
{
    ledcSetup(
        PWM_CHANNEL,
        PWM_FREQUENCY,
        PWM_RESOLUTION
    );

    ledcAttachPin(
        LED_PIN,
        PWM_CHANNEL
    );

    setBrightness(0);
}

void Lighting::setBrightness(uint8_t percent)
{
    percent = constrain(percent, 0, 100);

    brightness = percent;

    const uint32_t duty =
        map(percent, 0, 100, 0, 255);

    ledcWrite(
        PWM_CHANNEL,
        duty
    );
}

uint8_t Lighting::getBrightness() const
{
    return brightness;
}