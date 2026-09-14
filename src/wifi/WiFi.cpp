#include "WiFi.h"

#include <Arduino.h>
#include <WiFi.h>

namespace
{
    const char* WIFI_SSID = "Redmi_42F0";
    const char* WIFI_PASSWORD = "89250246180";
}

void setupWiFi()
{
    Serial.println("Connecting to Wi-Fi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi connected");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}