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