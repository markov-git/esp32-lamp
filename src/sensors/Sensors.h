#pragma once

#include "bme280/Bme280.h"
#include "soil_moisture/SoilMoisture.h"

struct SensorsState
{
    Bme280State bme280;
    SoilMoistureState soilMoisture;
};

class Sensors
{
public:
    bool begin();

    SensorsState getState();

private:
    Bme280 bme280;

    SoilMoisture soilMoisture;
};