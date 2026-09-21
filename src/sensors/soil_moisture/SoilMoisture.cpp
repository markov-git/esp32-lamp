#include "SoilMoisture.h"

bool SoilMoisture::begin()
{
    pinMode(SENSOR_PIN, INPUT);

    analogReadResolution(12);

    return true;
}

SoilMoistureState SoilMoisture::getState() const
{
    SoilMoistureState state{};

    state.raw = analogRead(SENSOR_PIN);

    return state;
}