#pragma once

#include <Arduino.h>
#include <RTClib.h>

#include "../sensors/Sensors.h"
#include "../storage/SdCard.h"

class History
{
public:
    explicit History(SdCard& sdCard);

    bool begin();

    void update(const DateTime& timestamp, const SensorsState& state);

    bool record(
        const DateTime& timestamp,
        const SensorsState& state
    );

private:
    static constexpr uint32_t RECORD_INTERVAL_SECONDS = 10 * 60;
    static constexpr const char* DIRECTORY = "/history";
    static constexpr const char* HEADER =
        "timestamp,temperature,humidity,pressure,"
        "soil1_raw,soil1_percent,"
        "soil2_raw,soil2_percent,"
        "soil3_raw,soil3_percent,crc\n";

    SdCard& sdCard;

    int64_t lastRecordTimestamp = -1;

    bool ensureDirectory();
    bool ensureFileHeader(const String& path);

    String getFilePath(const DateTime& timestamp) const;

    uint32_t calculateCrc32(const String& data) const;
};