#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Update.h>

#include "../controller/LightingController.h"
#include "../system/SystemInfo.h"
#include "../sensors/Sensors.h"
#include "../time/Rtc.h"
#include "../history/History.h"

class Api
{
public:
    explicit Api(
        LightingController& lightingController,
        SystemInfo& systemInfo,
        Sensors& sensors,
        Rtc& rtc,
        History& history
    );

    void registerRoutes(WebServer& server);

private:
    LightingController& lightingController;
    SystemInfo& systemInfo;
    Sensors& sensors;
    Rtc& rtc;
    History& history;

    bool firmwareUpdateStarted = false;
    bool firmwareUpdateSuccess = false;
    String firmwareUpdateError;
    void handleFirmwareUpload(WebServer& server);
    void handleFirmwareResult(WebServer& server);

    void handleGetHistory(WebServer& server);

    void handleState(WebServer& server);
    void handleSystem(WebServer& server);
    void handleSensors(WebServer& server);

    void handleTime(WebServer& server);
    void handleSetTime(WebServer& server);

    void handleTimezone(WebServer& server);
    void handleSetTimezone(WebServer& server);

    void handleSetManualBrightness(WebServer& server);

    void handleSetScheduleEnabled(WebServer& server);

    void handleSchedules(WebServer& server);
    void handleAddSchedule(WebServer& server);
    void handleUpdateSchedule(WebServer& server);
    void handleDeleteSchedule(WebServer& server);

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

    bool parseScheduleEntry(
        JsonVariantConst value,
        ScheduleEntry& entry
    );

    void sendJsonError(
        WebServer& server,
        int statusCode,
        const char* error
    );
};