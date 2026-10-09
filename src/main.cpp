#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>

#include "web/WebServerManager.h"
#include "wifi/WiFi.h"
#include "fs/FileServer.h"
#include "api/Api.h"
#include "lighting/Lighting.h"
#include "schedule/Schedule.h"
#include "system/SystemInfo.h"
#include "sensors/Sensors.h"
#include "time/Rtc.h"
#include "controller/LightingController.h"
#include "storage/SdCard.h"
#include "history/History.h"

WebServer server(80);

Lighting lighting;
Schedule schedule;
SystemInfo systemInfo;
Sensors sensors;
Rtc rtc;
SdCard sdCard;
History history(
    sdCard,
    sensors
);

LightingController lightingController(
    lighting,
    schedule,
    rtc
);

Api api(
    lightingController,
    systemInfo,
    sensors,
    rtc,
    history
);
WebServerManager webServer(server);

void setup() {
    Serial.begin(115200);

    setupFileServer();
    setupWiFi();

    lighting.begin();
    api.registerRoutes(server);
    webServer.begin();
    server.begin();

    sensors.begin();
    rtc.begin();
    schedule.load();
    sdCard.begin();
    if (sdCard.isReady())
    {
        history.begin();
    }
    lightingController.begin();

    Serial.println("App started");
}

constexpr uint32_t CONTROL_UPDATE_INTERVAL_MS = 1000;
uint32_t lastControlUpdateMs = 0;

void loop()
{
    const uint32_t currentMillis = millis();

    if (
        currentMillis - lastControlUpdateMs >=
        CONTROL_UPDATE_INTERVAL_MS
    )
    {
        lastControlUpdateMs +=
            CONTROL_UPDATE_INTERVAL_MS;

        const DateTime now =
            rtc.getDateTime();

        lightingController.update(now);
        history.update(now);
    }

    server.handleClient();
    
}