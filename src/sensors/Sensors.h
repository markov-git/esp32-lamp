#pragma once

#include "bme280/Bme280.h"

struct SensorsState
{
    Bme280State bme280;
};

class Sensors
{
public:
    void begin();

    SensorsState getState();

private:
    Bme280 bme280;
};