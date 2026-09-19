#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>

#include "../lighting/Lighting.h"
#include "../system/SystemInfo.h"
#include "../sensors/Sensors.h"

class Api
{
public:
    explicit Api(
        Lighting& lighting,
        SystemInfo& systemInfo,
        Sensors& sensors
    );

    void registerRoutes(WebServer& server);
    bool handleRequest(WebServer& server);

private:
    Lighting& lighting;
    SystemInfo& systemInfo;
    Sensors& sensors;

    void handleState(WebServer& server);
    void handleSystem(WebServer& server);
    void handleSensors(WebServer& server);

    void handleSetBrightness(
        WebServer& server,
        Lamp lamp,
        Channel channel
    );

    void sendState(WebServer& server);
    void sendSystem(WebServer& server);
    void sendSensors(WebServer& server);

    bool parseLamp(
        const String& value,
        Lamp& lamp
    );

    bool parseChannel(
        const String& value,
        Channel& channel
    );
};