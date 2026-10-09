#pragma once

#include "bme280/Bme280.h"
#include "soil_moisture/SoilMoisture.h"
#include "scd41/Scd41.h"

struct SensorsState
{
    Bme280State bme280;
    SoilMoistureState soilMoisture;
    Scd41State scd41;
};

class Sensors
{
public:
    bool begin();

    SensorsState getState();

private:
    Bme280 bme280;
    SoilMoisture soilMoisture;
    Scd41 scd41;
};