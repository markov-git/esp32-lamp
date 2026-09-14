#pragma once

#include <WebServer.h>

#include "../lighting/Lighting.h"

class Api
{
public:
    explicit Api(Lighting& lighting);

    void registerRoutes(WebServer& server);

private:
    Lighting& lighting;

    void handleState(WebServer& server);
    void handleSetBrightness(WebServer& server);
};