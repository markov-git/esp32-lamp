#pragma once

#include <WebServer.h>

#include "../lighting/Lighting.h"

class Api
{
public:
    explicit Api(Lighting& lighting);

    void registerRoutes(WebServer& server);
    bool handleRequest(WebServer& server);

private:
    Lighting& lighting;

    void handleState(WebServer& server);
    void handleSetBrightness(
        WebServer& server,
        Lamp lamp,
        Channel channel
    );

    bool parseLamp(
        const String& value,
        Lamp& lamp
    );

    bool parseChannel(
        const String& value,
        Channel& channel
    );
};