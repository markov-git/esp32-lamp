#include "Sensors.h"

bool Sensors::begin()
{
    delay(1000);
    const bool bme280Ready = bme280.begin();
    const bool soilMoistureReady = soilMoisture.begin();

    return bme280Ready && soilMoistureReady;
}

SensorsState Sensors::getState()
{
    SensorsState state{};

    state.bme280 = bme280.getState();
    state.soilMoisture = soilMoisture.getState();

    return state;
}