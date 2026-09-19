#include "Sensors.h"

void Sensors::begin()
{
    if (!bme280.begin())
    {
        Serial.println("Sensors initialization failed");
    }
    else
    {
        Serial.println("Sensors initialized");
    }
}

SensorsState Sensors::getState()
{
    SensorsState state{};

    state.bme280 = bme280.getState();

    return state;
}