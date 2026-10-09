#include "Sensors.h"

bool Sensors::begin()
{
    delay(1000);
    const bool bme280Ready = bme280.begin();
    const bool soilMoistureReady = soilMoisture.begin();
    const bool scd41Ready = scd41.begin();

    return bme280Ready && soilMoistureReady && scd41Ready;
}

SensorsState Sensors::getState()
{
    SensorsState state{};

    state.bme280 = bme280.getState();
    state.soilMoisture = soilMoisture.getState();
    state.scd41 = scd41.getState();

    return state;
}