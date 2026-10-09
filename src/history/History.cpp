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

    if (startTimestamp < 0)
    {
        startTimestamp = currentTimestamp;
        return;
    }

    if (currentTimestamp - startTimestamp <
        FIRST_RECORD_DELAY_SECONDS)
    {
        return;
    }

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

    char scd41Available[2] = "0";
    char scd41Co2[8] = "";
    char scd41Temperature[16] = "";
    char scd41Humidity[16] = "";

    if (state.scd41.available)
    {
        scd41Available[0] = '1';

        snprintf(
            scd41Co2,
            sizeof(scd41Co2),
            "%u",
            state.scd41.co2Ppm
        );

        snprintf(
            scd41Temperature,
            sizeof(scd41Temperature),
            "%.2f",
            state.scd41.temperature
        );

        snprintf(
            scd41Humidity,
            sizeof(scd41Humidity),
            "%.2f",
            state.scd41.humidity
        );
    }

    char data[256];

    snprintf(
        data,
        sizeof(data),
        "%lu,%.2f,%.2f,%.2f,"
        "%u,%u,%u,%u,%u,%u,"
        "%s,%s,%s,%s",
        static_cast<unsigned long>(timestamp.unixtime()),
        state.bme280.temperature,
        state.bme280.humidity,
        state.bme280.pressure,
        state.soilMoisture.raw[0],
        state.soilMoisture.percent[0],
        state.soilMoisture.raw[1],
        state.soilMoisture.percent[1],
        state.soilMoisture.raw[2],
        state.soilMoisture.percent[2],
        scd41Available,
        scd41Co2,
        scd41Temperature,
        scd41Humidity
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

    if (line.isEmpty() || line.length() >= sizeof(buffer))
        return false;

    line.toCharArray(buffer, sizeof(buffer));

    char* fields[15];
    uint8_t fieldCount = 0;
    char* fieldStart = buffer;

    // Разделяем строку вручную, сохраняя пустые поля.
    for (char* p = buffer; ; ++p)
    {
        if (*p == ',' || *p == '\0')
        {
            if (fieldCount >= 15)
                return false;

            fields[fieldCount++] = fieldStart;

            if (*p == '\0')
                break;

            *p = '\0';
            fieldStart = p + 1;
        }
    }

    if (fieldCount != 15)
        return false;

    char* end = nullptr;

    // timestamp
    const unsigned long timestamp =
        strtoul(fields[0], &end, 10);

    if (fields[0][0] == '\0' || *end != '\0' ||
        timestamp > UINT32_MAX)
        return false;

    record.timestamp = static_cast<uint32_t>(timestamp);

    // Температура, влажность и давление BME280.
    record.temperature = strtof(fields[1], &end);
    if (fields[1][0] == '\0' || *end != '\0' ||
        !isfinite(record.temperature))
        return false;

    record.humidity = strtof(fields[2], &end);
    if (fields[2][0] == '\0' || *end != '\0' ||
        !isfinite(record.humidity))
        return false;

    record.pressure = strtof(fields[3], &end);
    if (fields[3][0] == '\0' || *end != '\0' ||
        !isfinite(record.pressure))
        return false;

    // Вспомогательный разбор целого числа с проверкой диапазона.
    auto parseUnsigned = [](const char* text,
        unsigned long maxValue,
        unsigned long& value) -> bool
        {
            if (text[0] == '\0' || text[0] == '-')
                return false;

            char* end = nullptr;
            value = strtoul(text, &end, 10);

            return end != text &&
                *end == '\0' &&
                value <= maxValue;
        };

    unsigned long value = 0;

    if (!parseUnsigned(fields[4], UINT16_MAX, value))
        return false;
    record.soilRaw[0] = static_cast<uint16_t>(value);

    if (!parseUnsigned(fields[5], 100, value))
        return false;
    record.soilPercent[0] = static_cast<uint8_t>(value);

    if (!parseUnsigned(fields[6], UINT16_MAX, value))
        return false;
    record.soilRaw[1] = static_cast<uint16_t>(value);

    if (!parseUnsigned(fields[7], 100, value))
        return false;
    record.soilPercent[1] = static_cast<uint8_t>(value);

    if (!parseUnsigned(fields[8], UINT16_MAX, value))
        return false;
    record.soilRaw[2] = static_cast<uint16_t>(value);

    if (!parseUnsigned(fields[9], 100, value))
        return false;
    record.soilPercent[2] = static_cast<uint8_t>(value);

    // SCD41: доступность должна быть строго 0 или 1.
    if (strcmp(fields[10], "0") == 0)
    {
        record.scd41Available = false;
        record.scd41Co2Ppm = 0;
        record.scd41Temperature = NAN;
        record.scd41Humidity = NAN;

        // При недоступном датчике измерения обязаны быть пустыми.
        if (fields[11][0] != '\0' ||
            fields[12][0] != '\0' ||
            fields[13][0] != '\0')
            return false;
    }
    else if (strcmp(fields[10], "1") == 0)
    {
        record.scd41Available = true;

        if (!parseUnsigned(fields[11], UINT16_MAX, value) ||
            value == 0)
            return false;

        record.scd41Co2Ppm = static_cast<uint16_t>(value);

        record.scd41Temperature = strtof(fields[12], &end);
        if (fields[12][0] == '\0' || *end != '\0' ||
            !isfinite(record.scd41Temperature))
            return false;

        record.scd41Humidity = strtof(fields[13], &end);
        if (fields[13][0] == '\0' || *end != '\0' ||
            !isfinite(record.scd41Humidity) ||
            record.scd41Humidity < 0.0f ||
            record.scd41Humidity > 100.0f)
            return false;
    }
    else
    {
        return false;
    }

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