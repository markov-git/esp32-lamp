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

    void handleSetManualBrightness(WebServer& server);
    void handleSetScheduleEnabled(WebServer& server);

    void sendState(WebServer& server);
    void sendSystem(WebServer& server);
    void sendSensors(WebServer& server);
    void sendTime(WebServer& server);

    bool parseJsonBody(
        WebServer& server,
        JsonDocument& doc
    );

    bool parseLamp(
        JsonVariantConst value,
        Lamp& lamp
    ) const;

    bool parseChannel(
        JsonVariantConst value,
        Channel& channel
    ) const;

    void sendJsonError(
        WebServer& server,
        int statusCode,
        const char* error
    );
};