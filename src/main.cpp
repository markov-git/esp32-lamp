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

WebServer server(80);

Lighting lighting;
Schedule schedule;
SystemInfo systemInfo;
Sensors sensors;
Rtc rtc;

LightingController lightingController(
    lighting,
    schedule,
    rtc
);

Api api(
    lightingController,
    systemInfo,
    sensors,
    rtc
);
WebServerManager webServer;

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
    lightingController.begin();

    Serial.println("App started");
}

void loop()
{
    webServer.handleClient();

    lightingController.update();
}