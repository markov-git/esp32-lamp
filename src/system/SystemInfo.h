#pragma once

#include <Arduino.h>

struct SystemInfoState
{
    // Device
    String chipModel;
    uint8_t chipRevision;
    uint8_t cpuCores;
    uint32_t cpuFrequencyMhz;

    // Runtime
    uint64_t uptimeSeconds;
    uint32_t freeHeap;
    uint32_t totalHeap;
    uint32_t minimumFreeHeap;

    // Flash
    uint32_t flashSize;
    uint32_t sketchSize;
    uint32_t freeSketchSpace;

    // Filesystem
    uint32_t filesystemTotal;
    uint32_t filesystemUsed;
    uint32_t filesystemFree;

    // Wi-Fi
    String ip;
    String gateway;
    String subnet;
    String mac;
    int32_t wifiRssi;

    // ESP32 chip temperature
    float chipTemperature;
};

class SystemInfo
{
public:
    SystemInfoState getState() const;

private:
    float getChipTemperature() const;
};