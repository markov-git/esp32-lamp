#include "SystemInfo.h"

#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <esp_timer.h>
#include <esp_system.h>
#include <esp_chip_info.h>

SystemInfoState SystemInfo::getState() const
{
    SystemInfoState state{};

    // -------------------------
    // Device
    // -------------------------

    state.chipModel = ESP.getChipModel();
    state.chipRevision = ESP.getChipRevision();
    state.cpuCores = ESP.getChipCores();
    state.cpuFrequencyMhz = ESP.getCpuFreqMHz();

    // -------------------------
    // Runtime
    // -------------------------

    state.uptimeSeconds = esp_timer_get_time() / 1000000ULL;

    state.freeHeap = ESP.getFreeHeap();
    state.totalHeap = ESP.getHeapSize();
    state.minimumFreeHeap = ESP.getMinFreeHeap();

    // -------------------------
    // Flash
    // -------------------------

    state.flashSize = ESP.getFlashChipSize();
    state.sketchSize = ESP.getSketchSize();
    state.freeSketchSpace = ESP.getFreeSketchSpace();

    // -------------------------
    // LittleFS
    // -------------------------

    state.filesystemTotal = LittleFS.totalBytes();
    state.filesystemUsed = LittleFS.usedBytes();
    state.filesystemFree =
        state.filesystemTotal -
        state.filesystemUsed;

    // -------------------------
    // Wi-Fi
    // -------------------------

    state.ip = WiFi.localIP().toString();
    state.gateway = WiFi.gatewayIP().toString();
    state.subnet = WiFi.subnetMask().toString();
    state.mac = WiFi.macAddress();
    state.wifiRssi = WiFi.RSSI();

    // -------------------------
    // Temperature
    // -------------------------

    state.chipTemperature = getChipTemperature();

    return state;
}

float SystemInfo::getChipTemperature() const
{
    return temperatureRead();
}