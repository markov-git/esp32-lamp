#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>

#include "web/WebServerManager.h"
#include "wifi/WiFi.h"
#include "fs/FileServer.h"
#include "api/Api.h"
#include "lighting/Lighting.h"
#include "system/SystemInfo.h"
#include "sensors/Sensors.h"
#include "time/Rtc.h"

WebServer server(80);

Lighting lighting1;
SystemInfo systemInfo;
Sensors sensors;
Rtc rtc;

Api api(lighting1, systemInfo, sensors, rtc);
WebServerManager webServer(api);

void setup() {
    Serial.begin(115200);

    setupFileServer();
    setupWiFi();
    rtc.begin();
    lighting1.begin();
    webServer.begin();
    sensors.begin();

    Serial.println("App started");
}

void loop()
{
    webServer.handleClient();
}