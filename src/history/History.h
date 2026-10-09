#pragma once

#include <Arduino.h>
#include <RTClib.h>

#include "../sensors/Sensors.h"
#include "../storage/SdCard.h"

enum class HistoryRange
{
    Day,
    Month,
    All
};

struct HistoryRecord
{
    uint32_t timestamp;

    float temperature;
    float humidity;
    float pressure;

    uint16_t soilRaw[3];
    uint8_t soilPercent[3];

    bool scd41Available = false;
    uint16_t scd41Co2Ppm = 0;
    float scd41Temperature = NAN;
    float scd41Humidity = NAN;
};

using HistoryRecordCallback =
    void (*)(const HistoryRecord& record, void* context);

class History
{
public:
    History(
        SdCard& sdCard,
        Sensors& sensors
    );

    bool begin();

    void update(const DateTime& timestamp);

    bool forEachRecord(
        HistoryRange range,
        const DateTime& now,
        HistoryRecordCallback callback,
        void* context
    ) const;

    bool readAllRecords(
        HistoryRecordCallback callback,
        void* context
    ) const;

private:
    static constexpr uint16_t MAX_HISTORY_FILES = 4000;
    static constexpr uint8_t HISTORY_PATH_LENGTH = 32;

    static constexpr uint32_t RECORD_INTERVAL_SECONDS = 10 * 60;
    static constexpr const char* DIRECTORY = "/history";
    static constexpr const char* HEADER =
        "timestamp,temperature,humidity,pressure,"
        "soil1_raw,soil1_percent,"
        "soil2_raw,soil2_percent,"
        "soil3_raw,soil3_percent,"
        "scd41_available,scd41_co2_ppm,"
        "scd41_temperature,scd41_humidity,"
        "crc\n";

    SdCard& sdCard;
    Sensors& sensors;

    int64_t lastAttemptTimestamp = -1;

    bool isValidState(const SensorsState& state) const;

    bool record(
        const DateTime& timestamp,
        const SensorsState& state
    );

    bool ensureDirectory();
    bool ensureFileHeader(const String& path);

    String getFilePath(const DateTime& timestamp) const;

    uint32_t calculateCrc32(const String& data) const;

    bool readFileRecords(
        const String& path,
        HistoryRecordCallback callback,
        void* context
    ) const;

    bool parseRecord(
        const String& line,
        HistoryRecord& record
    ) const;

    bool isRecordCrcValid(
        const String& line
    ) const;
};