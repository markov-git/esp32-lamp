#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>

#include "../lighting/Lighting.h"
#include "../system/SystemInfo.h"

class Api
{
public:
    explicit Api(
        Lighting& lighting,
        SystemInfo& systemInfo
    );

    void registerRoutes(WebServer& server);
    bool handleRequest(WebServer& server);

private:
    Lighting& lighting;
    SystemInfo& systemInfo;

    void handleState(WebServer& server);
    void handleSystem(WebServer& server);

    void handleSetBrightness(
        WebServer& server,
        Lamp lamp,
        Channel channel
    );

    void sendState(WebServer& server);
    void sendSystem(WebServer& server);

    bool parseLamp(
        const String& value,
        Lamp& lamp
    );

    bool parseChannel(
        const String& value,
        Channel& channel
    );
};