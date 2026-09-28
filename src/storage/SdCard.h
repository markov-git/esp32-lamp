#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class SdCard
{
public:
    bool begin();
    bool isReady() const;

    bool exists(const char* path) const;

    File open(const char* path, const char* mode = FILE_READ);

    bool writeFile(
        const char* path,
        const String& content
    );

    bool appendFile(
        const char* path,
        const String& content
    );

    bool readFile(
        const char* path,
        String& content
    );

    bool removeFile(const char* path);

private:
    static constexpr uint8_t CS_PIN = 5;
    static constexpr uint8_t SCK_PIN = 18;
    static constexpr uint8_t MISO_PIN = 19;
    static constexpr uint8_t MOSI_PIN = 23;

    bool ready = false;
};