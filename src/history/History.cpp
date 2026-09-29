#include <algorithm>
#include <vector>

#include "History.h"

History::History(
    SdCard& sdCard,
    Sensors& sensors
)
    : sdCard(sdCard),
      sensors(sensors)
{
}

bool History::begin()
{
    if (!sdCard.isReady())
    {
        Serial.println("History: SD card is not ready.");
        return false;
    }

    if (!ensureDirectory())
    {
        Serial.println("History: failed to create directory.");
        return false;
    }

    return true;
}

void History::update(
    const DateTime& timestamp
)
{
    const int64_t currentTimestamp =
        static_cast<int64_t>(timestamp.unixtime());

    if (
        lastAttemptTimestamp >= 0 &&
        currentTimestamp - lastAttemptTimestamp <
            RECORD_INTERVAL_SECONDS
    )
    {
        return;
    }

    lastAttemptTimestamp = currentTimestamp;

    const SensorsState state =
        sensors.getState();

    if (!isValidState(state))
    {
        Serial.println(
            "History: invalid sensor state, "
            "record skipped"
        );

        return;
    }

    if (!record(timestamp, state))
    {
        Serial.println(
            "History: failed to record data"
        );

        return;
    }
}

bool History::record(
    const DateTime& timestamp,
    const SensorsState& state
)
{
    if (!sdCard.isReady())
        return false;

    const String path = getFilePath(timestamp);

    if (!ensureFileHeader(path))
        return false;

    char data[192];

    snprintf(
        data,
        sizeof(data),
        "%lu,%.2f,%.2f,%.2f,"
        "%u,%u,"
        "%u,%u,"
        "%u,%u",
        static_cast<unsigned long>(timestamp.unixtime()),
        state.bme280.temperature,
        state.bme280.humidity,
        state.bme280.pressure,
        state.soilMoisture.raw[0],
        state.soilMoisture.percent[0],
        state.soilMoisture.raw[1],
        state.soilMoisture.percent[1],
        state.soilMoisture.raw[2],
        state.soilMoisture.percent[2]
    );

    const String dataString(data);

    const uint32_t crc = calculateCrc32(dataString);

    char crcText[9];

    snprintf(
        crcText,
        sizeof(crcText),
        "%08lX",
        static_cast<unsigned long>(crc)
    );

    File file = sdCard.open(
        path.c_str(),
        FILE_APPEND
    );

    if (!file)
    {
        Serial.print("History: failed to open file: ");
        Serial.println(path);

        return false;
    }

    const size_t dataWritten = file.print(dataString);

    if (dataWritten != dataString.length())
    {
        file.close();
        return false;
    }

    if (file.print(',') != 1)
    {
        file.close();
        return false;
    }

    if (file.print(crcText) != strlen(crcText))
    {
        file.close();
        return false;
    }

    if (file.print('\n') != 1)
    {
        file.close();
        return false;
    }

    file.close();

    return true;
}

bool History::ensureFileHeader(const String& path)
{
    if (sdCard.exists(path.c_str()))
        return true;

    File file = sdCard.open(
        path.c_str(),
        FILE_WRITE
    );

    if (!file)
    {
        Serial.print("History: failed to create file: ");
        Serial.println(path);

        return false;
    }

    const size_t written = file.print(HEADER);

    file.close();

    if (written != strlen(HEADER))
    {
        Serial.print("History: failed to write header: ");
        Serial.println(path);

        return false;
    }

    return true;
}

String History::getFilePath(
    const DateTime& timestamp
) const
{
    char filename[32];

    snprintf(
        filename,
        sizeof(filename),
        "%s/%04d-%02d-%02d.csv",
        DIRECTORY,
        timestamp.year(),
        timestamp.month(),
        timestamp.day()
    );

    return String(filename);
}

bool History::isValidState(const SensorsState& state) const
{
    bool valid = true;

    if (!isfinite(state.bme280.temperature))
    {
        Serial.println("History: invalid temperature");
        valid = false;
    }

    if (!isfinite(state.bme280.humidity))
    {
        Serial.println("History: invalid humidity");
        valid = false;
    }

    if (!isfinite(state.bme280.pressure))
    {
        Serial.println("History: invalid pressure");
        valid = false;
    }

    for (uint8_t i = 0; i < SoilMoisture::SENSOR_COUNT; i++)
    {
        if (state.soilMoisture.percent[i] > 100)
        {
            Serial.print(
                "History: invalid soil moisture percent, sensor "
            );
            Serial.println(i + 1);

            valid = false;
        }
    }

    return valid;
}

uint32_t History::calculateCrc32(
    const String& data
) const
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < data.length(); i++)
    {
        crc ^= static_cast<uint8_t>(data[i]);

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return ~crc;
}

bool History::ensureDirectory()
{
    return sdCard.mkdir(DIRECTORY);
}

bool History::forEachRecord(
    HistoryRange range,
    const DateTime& now,
    HistoryRecordCallback callback,
    void* context
) const
{
    if (!sdCard.isReady())
        return false;

    if (callback == nullptr)
        return false;

    if (range == HistoryRange::Day)
    {
        const DateTime yesterday =
            now - TimeSpan(1, 0, 0, 0);

        const String yesterdayPath =
            getFilePath(yesterday);

        if (sdCard.exists(yesterdayPath.c_str()))
        {
            readFileRecords(
                yesterdayPath,
                callback,
                context
            );
        }

        const String todayPath =
            getFilePath(now);

        if (sdCard.exists(todayPath.c_str()))
        {
            readFileRecords(
                todayPath,
                callback,
                context
            );
        }

        return true;
    }

    std::vector<String> paths;

    if (!sdCard.listFiles(
            DIRECTORY,
            paths))
    {
        Serial.println(
            "History: failed to list history files"
        );

        return false;
    }

    std::sort(
        paths.begin(),
        paths.end()
    );

    if (range == HistoryRange::All)
    {
        for (const String& path : paths)
        {
            if (!path.endsWith(".csv"))
                continue;

            readFileRecords(
                path,
                callback,
                context
            );
        }

        return true;
    }

    // Month:
    const DateTime firstDay =
        now - TimeSpan(30, 0, 0, 0);

    const String firstPath =
        getFilePath(firstDay);

    const String lastPath =
        getFilePath(now);

    for (const String& path : paths)
    {
        if (!path.endsWith(".csv"))
            continue;

        if (path < firstPath)
            continue;

        if (path > lastPath)
            continue;

        readFileRecords(
            path,
            callback,
            context
        );
    }

    return true;
}

bool History::readFileRecords(
    const String& path,
    HistoryRecordCallback callback,
    void* context
) const
{
    if (!sdCard.isReady())
        return false;

    File file = sdCard.open(path.c_str(), FILE_READ);

    if (!file)
    {
        Serial.print("History: failed to open file: ");
        Serial.println(path);

        return false;
    }

    // Skip CSV header.
    file.readStringUntil('\n');

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.isEmpty())
            continue;

        HistoryRecord record{};

        if (!parseRecord(line, record))
        {
            Serial.print(
                "History: invalid record in "
            );
            Serial.println(path);

            continue;
        }

        if (!isRecordCrcValid(line))
        {
            Serial.print(
                "History: CRC mismatch in "
            );
            Serial.println(path);

            continue;
        }

        callback(record, context);
    }

    file.close();

    return true;
}

bool History::parseRecord(
    const String& line,
    HistoryRecord& record
) const
{
    char buffer[256];

    if (line.length() >= sizeof(buffer))
        return false;

    line.toCharArray(buffer, sizeof(buffer));

    char* fields[11];

    uint8_t fieldCount = 0;

    char* token = strtok(buffer, ",");

    while (token != nullptr && fieldCount < 11)
    {
        fields[fieldCount++] = token;
        token = strtok(nullptr, ",");
    }

    if (fieldCount != 11)
        return false;

    char* end = nullptr;

    const unsigned long timestamp =
        strtoul(fields[0], &end, 10);

    if (*end != '\0')
        return false;

    record.timestamp =
        static_cast<uint32_t>(timestamp);

    record.temperature =
        strtof(fields[1], &end);

    if (*end != '\0')
        return false;

    record.humidity =
        strtof(fields[2], &end);

    if (*end != '\0')
        return false;

    record.pressure =
        strtof(fields[3], &end);

    if (*end != '\0')
        return false;

    record.soilRaw[0] =
        static_cast<uint16_t>(
            strtoul(fields[4], &end, 10)
        );

    if (*end != '\0')
        return false;

    record.soilPercent[0] =
        static_cast<uint8_t>(
            strtoul(fields[5], &end, 10)
        );

    if (*end != '\0')
        return false;

    record.soilRaw[1] =
        static_cast<uint16_t>(
            strtoul(fields[6], &end, 10)
        );

    if (*end != '\0')
        return false;

    record.soilPercent[1] =
        static_cast<uint8_t>(
            strtoul(fields[7], &end, 10)
        );

    if (*end != '\0')
        return false;

    record.soilRaw[2] =
        static_cast<uint16_t>(
            strtoul(fields[8], &end, 10)
        );

    if (*end != '\0')
        return false;

    record.soilPercent[2] =
        static_cast<uint8_t>(
            strtoul(fields[9], &end, 10)
        );

    if (*end != '\0')
        return false;

    return true;
}

bool History::isRecordCrcValid(
    const String& line
) const
{
    const int separator = line.lastIndexOf(',');

    if (separator < 0)
        return false;

    const String data =
        line.substring(0, separator);

    const String crcText =
        line.substring(separator + 1);

    if (crcText.length() != 8)
        return false;

    const uint32_t expectedCrc =
        strtoul(crcText.c_str(), nullptr, 16);

    const uint32_t actualCrc =
        calculateCrc32(data);

    return expectedCrc == actualCrc;
}

bool History::readAllRecords(
    HistoryRecordCallback callback,
    void* context
) const
{
    std::vector<String> paths;

    if (!sdCard.listFiles(
            DIRECTORY,
            paths))
    {
        Serial.println(
            "History: failed to list history files"
        );

        return false;
    }

    std::sort(
        paths.begin(),
        paths.end()
    );

    for (const String& path : paths)
    {
        if (!path.endsWith(".csv"))
            continue;

        if (!readFileRecords(
                path,
                callback,
                context))
        {
            Serial.print(
                "History: failed to read file: "
            );
            Serial.println(path);

            // ошибки файла логируем
            // остальные файлы продолжаем читать
            continue;
        }
    }

    return true;
}