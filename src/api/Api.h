#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>

#include "../lighting/Lighting.h"
#include "../system/SystemInfo.h"
#include "../sensors/Sensors.h"
#include "../time/Rtc.h"

class Api
{
public:
    explicit Api(
        Lighting& lighting,
        SystemInfo& systemInfo,
        Sensors& sensors,
        Rtc& rtc
    );

    void registerRoutes(WebServer& server);
    bool handleRequest(WebServer& server);

private:
    Lighting& lighting;
    SystemInfo& systemInfo;
    Sensors& sensors;
    Rtc& rtc;

    void handleState(WebServer& server);
    void handleSystem(WebServer& server);
    void handleSensors(WebServer& server);
    void handleTime(WebServer& server);
    
    void handleSetBrightness(
        WebServer& server,
        Lamp lamp,
        Channel channel
    );
    void handleSetTime(WebServer& server);

    void sendState(WebServer& server);
    void sendSystem(WebServer& server);
    void sendSensors(WebServer& server);
    void sendTime(WebServer& server);

    bool parseLamp(
        const String& value,
        Lamp& lamp
    );

    bool parseChannel(
        const String& value,
        Channel& channel
    );
};