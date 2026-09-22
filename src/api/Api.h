#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>
#include <Arduino.h>

#include "../controller/LightingController.h"
#include "../system/SystemInfo.h"
#include "../sensors/Sensors.h"
#include "../time/Rtc.h"

class Api
{
public:
    explicit Api(
        LightingController& lightingController,
        SystemInfo& systemInfo,
        Sensors& sensors,
        Rtc& rtc
    );

    void registerRoutes(WebServer& server);
    bool handleRequest(WebServer& server);

private:
    LightingController& lightingController;
    SystemInfo& systemInfo;
    Sensors& sensors;
    Rtc& rtc;

    void handleState(WebServer& server);
    void handleSystem(WebServer& server);
    void handleSensors(WebServer& server);
    void handleTime(WebServer& server);
    
    void handleSetTime(WebServer& server);

    void sendState(WebServer& server);
    void sendSystem(WebServer& server);
    void sendSensors(WebServer& server);
    void sendTime(WebServer& server);

    void handleSetBrightness(
        WebServer& server,
        Lamp lamp,
        Channel channel
    );

    void handleSetScheduleEnabled(
        WebServer& server,
        Lamp lamp
    );

    bool parseLamp(
        const String& value,
        Lamp& lamp
    ) const;

    bool parseChannel(
        const String& value,
        Channel& channel
    ) const;

    bool parseLampChannel(
        const String& path,
        Lamp& lamp,
        Channel& channel
    ) const;

    bool parseLampSchedule(
        const String& path,
        Lamp& lamp
    ) const;

    void sendJsonError(
        WebServer& server,
        int statusCode,
        const char* error
    );
};