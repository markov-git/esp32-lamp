#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>

#include "web/WebServerManager.h"
#include "wifi/WiFi.h"
#include "lighting/Lighting.h"
#include "fs/FileServer.h"
#include "api/Api.h"

WebServer server(80);

Lighting lighting1;
Api api(lighting1);
WebServerManager webServer(api);

void setup() {
    Serial.begin(115200);

    setupFileServer();
    setupWiFi();
    lighting1.begin();
    webServer.begin();

    Serial.println("App started");
}

void loop()
{
    webServer.handleClient();
}