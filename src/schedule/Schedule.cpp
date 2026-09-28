#include "Schedule.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

namespace
{
    constexpr uint8_t DAYS_MASK = 0b01111111;
    constexpr uint16_t MINUTES_PER_DAY = 24 * 60;

    constexpr char SCHEDULE_FILE[] =
        "/config/schedule.json";

    constexpr char TEMP_FILE[] =
        "/config/schedule.tmp";

    bool loadChannel(
        JsonVariantConst value,
        ScheduleEntry* entries,
        uint8_t& count
    )
    {
        if (!value.is<JsonArrayConst>())
            return false;

        JsonArrayConst array =
            value.as<JsonArrayConst>();

        if (
            array.size() >
            Schedule::MAX_ENTRIES_PER_CHANNEL
        )
        {
            return false;
        }

        count = 0;

        for (JsonObjectConst object : array)
        {
            if (
                !object["days"].is<int>() ||
                !object["start"].is<int>() ||
                !object["end"].is<int>() ||
                !object["brightness"].is<int>() ||
                !object["fadeIn"].is<int>() ||
                !object["fadeOut"].is<int>()
            )
            {
                return false;
            }

            const int days =
                object["days"].as<int>();

            const int start =
                object["start"].as<int>();

            const int end =
                object["end"].as<int>();

            const int brightness =
                object["brightness"].as<int>();

            const int fadeIn =
                object["fadeIn"].as<int>();

            const int fadeOut =
                object["fadeOut"].as<int>();

            if (
                days < 0 || days > 127 ||
                start < 0 || start > 1440 ||
                end < 0 || end > 1440 ||
                brightness < 0 || brightness > 100 ||
                fadeIn < 0 || fadeIn > 1440 ||
                fadeOut < 0 || fadeOut > 1440
            )
            {
                return false;
            }

            ScheduleEntry& entry =
                entries[count];

            entry.days =
                static_cast<uint8_t>(days);

            entry.startMinute =
                static_cast<uint16_t>(start);

            entry.endMinute =
                static_cast<uint16_t>(end);

            entry.brightness =
                static_cast<uint8_t>(brightness);

            entry.fadeInMinutes =
                static_cast<uint16_t>(fadeIn);

            entry.fadeOutMinutes =
                static_cast<uint16_t>(fadeOut);

            count++;
        }

        return true;
    }
}

Schedule::Schedule()
{
    clear();
}

bool Schedule::save() const
{
    if (!LittleFS.exists("/config"))
    {
        if (!LittleFS.mkdir("/config"))
        {
            Serial.println(
                "Failed to create config directory"
            );

            return false;
        }
    }

    JsonDocument doc;

    doc["timezoneOffsetMinutes"] =
        timezoneOffsetMinutes;

    JsonArray lamps =
        doc["lamps"].to<JsonArray>();

    for (uint8_t i = 0;
         i < Lighting::LAMP_COUNT;
         i++)
    {
        const LampSchedule& schedule =
            schedules[i];

        JsonObject lamp =
            lamps.add<JsonObject>();

        lamp["enabled"] =
            schedule.enabled;

        JsonArray red =
            lamp["red"].to<JsonArray>();

        for (uint8_t j = 0;
             j < schedule.redCount;
             j++)
        {
            const ScheduleEntry& source =
                schedule.red[j];

            JsonObject entry =
                red.add<JsonObject>();

            entry["days"] =
                source.days;

            entry["start"] =
                source.startMinute;

            entry["end"] =
                source.endMinute;

            entry["brightness"] =
                source.brightness;

            entry["fadeIn"] =
                source.fadeInMinutes;

            entry["fadeOut"] =
                source.fadeOutMinutes;
        }

        JsonArray blue =
            lamp["blue"].to<JsonArray>();

        for (uint8_t j = 0;
             j < schedule.blueCount;
             j++)
        {
            const ScheduleEntry& source =
                schedule.blue[j];

            JsonObject entry =
                blue.add<JsonObject>();

            entry["days"] =
                source.days;

            entry["start"] =
                source.startMinute;

            entry["end"] =
                source.endMinute;

            entry["brightness"] =
                source.brightness;

            entry["fadeIn"] =
                source.fadeInMinutes;

            entry["fadeOut"] =
                source.fadeOutMinutes;
        }
    }

    File file =
        LittleFS.open(TEMP_FILE, "w");

    if (!file)
    {
        Serial.println(
            "Failed to open schedule temp file"
        );

        return false;
    }

    const size_t bytesWritten =
        serializeJson(doc, file);

    file.close();

    if (bytesWritten == 0)
    {
        Serial.println(
            "Failed to write schedule"
        );

        LittleFS.remove(TEMP_FILE);

        return false;
    }

    if (LittleFS.exists(SCHEDULE_FILE))
    {
        LittleFS.remove(SCHEDULE_FILE);
    }

    if (!LittleFS.rename(
        TEMP_FILE,
        SCHEDULE_FILE
    ))
    {
        Serial.println(
            "Failed to replace schedule file"
        );

        LittleFS.remove(TEMP_FILE);

        return false;
    }

    return true;
}

bool Schedule::load()
{
    if (!LittleFS.exists(SCHEDULE_FILE))
    {
        Serial.println(
            "Schedule file not found, using defaults"
        );

        clear();

        return true;
    }

    File file =
        LittleFS.open(SCHEDULE_FILE, "r");

    if (!file)
    {
        Serial.println(
            "Failed to open schedule file"
        );

        clear();

        return false;
    }

    JsonDocument doc;

    const DeserializationError error =
        deserializeJson(doc, file);

    file.close();

    if (error)
    {
        Serial.print(
            "Failed to parse schedule: "
        );

        Serial.println(
            error.c_str()
        );

        clear();

        return false;
    }

    clear();

    if (
        doc["timezoneOffsetMinutes"]
            .is<int>()
    )
    {
        timezoneOffsetMinutes =
            doc["timezoneOffsetMinutes"]
                .as<int16_t>();
    }

    JsonArrayConst lamps =
        doc["lamps"].as<JsonArrayConst>();

    if (lamps.isNull())
    {
        Serial.println(
            "Invalid schedule: lamps missing"
        );

        clear();

        return false;
    }

    if (lamps.size() != Lighting::LAMP_COUNT)
    {
        Serial.println(
            "Invalid schedule: wrong lamp count"
        );

        clear();

        return false;
    }

    for (uint8_t i = 0;
         i < Lighting::LAMP_COUNT;
         i++)
    {
        JsonObjectConst lamp =
            lamps[i].as<JsonObjectConst>();

        if (lamp.isNull())
        {
            clear();

            return false;
        }

        schedules[i].enabled =
            lamp["enabled"] | false;

        if (!loadChannel(
                lamp["red"],
                schedules[i].red,
                schedules[i].redCount
            ))
        {
            clear();

            return false;
        }

        if (!loadChannel(
                lamp["blue"],
                schedules[i].blue,
                schedules[i].blueCount
            ))
        {
            clear();

            return false;
        }
    }

    return true;
}

void Schedule::clear()
{
    for (uint8_t i = 0; i < Lighting::LAMP_COUNT; i++)
    {
        schedules[i].enabled = false;

        schedules[i].redCount = 0;
        schedules[i].blueCount = 0;
    }

    timezoneOffsetMinutes = 180;
}

ScheduleError Schedule::addEntry(
    Lamp lamp,
    Channel channel,
    const ScheduleEntry& entry
)
{
    if (entry.days == 0 || entry.days > 127)
        return ScheduleError::InvalidDays;

    if (
        entry.startMinute >= entry.endMinute ||
        entry.endMinute > 1440
    )
    {
        return ScheduleError::InvalidTime;
    }

    if (entry.brightness > 100)
        return ScheduleError::InvalidBrightness;

    const uint16_t duration =
        entry.endMinute - entry.startMinute;

    if (
        entry.fadeInMinutes > duration ||
        entry.fadeOutMinutes > duration
    )
    {
        return ScheduleError::InvalidFade;
    }

    const uint8_t lampIndex = getLampIndex(lamp);

    ScheduleEntry* entries = nullptr;
    uint8_t* count = nullptr;

    if (channel == Channel::Red)
    {
        entries = schedules[lampIndex].red;
        count = &schedules[lampIndex].redCount;
    }
    else
    {
        entries = schedules[lampIndex].blue;
        count = &schedules[lampIndex].blueCount;
    }

    if (*count >= MAX_ENTRIES_PER_CHANNEL)
        return ScheduleError::MaxEntries;

    for (uint8_t i = 0; i < *count; i++)
    {
        if (
            (entry.days & entries[i].days) != 0 &&
            hasOverlap(entry, entries[i])
        )
        {
            return ScheduleError::Overlap;
        }
    }

    entries[*count] = entry;
    (*count)++;

    return ScheduleError::None;
}
ScheduleError Schedule::updateEntry(
    Lamp lamp,
    Channel channel,
    uint8_t index,
    const ScheduleEntry& entry
)
{
    const uint8_t lampIndex = getLampIndex(lamp);

    ScheduleEntry* entries = nullptr;
    uint8_t* count = nullptr;

    if (channel == Channel::Red)
    {
        entries = schedules[lampIndex].red;
        count = &schedules[lampIndex].redCount;
    }
    else
    {
        entries = schedules[lampIndex].blue;
        count = &schedules[lampIndex].blueCount;
    }

    if (index >= *count)
        return ScheduleError::EntryNotFound;

    if (entry.days == 0 || entry.days > 127)
        return ScheduleError::InvalidDays;

    if (
        entry.startMinute >= entry.endMinute ||
        entry.endMinute > 1440
    )
    {
        return ScheduleError::InvalidTime;
    }

    if (entry.brightness > 100)
        return ScheduleError::InvalidBrightness;

    const uint16_t duration =
        entry.endMinute - entry.startMinute;

    if (
        entry.fadeInMinutes > duration ||
        entry.fadeOutMinutes > duration
    )
    {
        return ScheduleError::InvalidFade;
    }

    for (uint8_t i = 0; i < *count; i++)
    {
        if (i == index)
            continue;

        if (
            (entry.days & entries[i].days) != 0 &&
            hasOverlap(entry, entries[i])
        )
        {
            return ScheduleError::Overlap;
        }
    }

    entries[index] = entry;

    return ScheduleError::None;
}
bool Schedule::removeEntry(
    Lamp lamp,
    Channel channel,
    uint8_t index
)
{
    const uint8_t lampIndex = getLampIndex(lamp);

    ScheduleEntry* entries = nullptr;
    uint8_t* count = nullptr;

    if (channel == Channel::Red)
    {
        entries = schedules[lampIndex].red;
        count = &schedules[lampIndex].redCount;
    }
    else
    {
        entries = schedules[lampIndex].blue;
        count = &schedules[lampIndex].blueCount;
    }

    if (index >= *count)
        return false;

    for (uint8_t i = index; i + 1 < *count; i++)
    {
        entries[i] = entries[i + 1];
    }

    (*count)--;

    return true;
}

void Schedule::setEnabled(
    Lamp lamp,
    bool enabled
)
{
    schedules[getLampIndex(lamp)].enabled = enabled;
}

bool Schedule::isEnabled(Lamp lamp) const
{
    return schedules[getLampIndex(lamp)].enabled;
}

const LampSchedule& Schedule::getSchedule(Lamp lamp) const
{
    return schedules[getLampIndex(lamp)];
}

ScheduleState Schedule::getState(
    const DateTime& now
) const
{
    ScheduleState state{};

    const DateTime localTime(
        now.unixtime() +
        static_cast<int32_t>(timezoneOffsetMinutes) * 60
    );

    const uint16_t currentMinute =
        localTime.hour() * 60 +
        localTime.minute();

    const uint8_t dayOfWeek =
        now.dayOfTheWeek();

    for (uint8_t lampIndex = 0;
         lampIndex < Lighting::LAMP_COUNT;
         lampIndex++)
    {
        const LampSchedule& schedule =
            schedules[lampIndex];

        if (!schedule.enabled)
        {
            continue;
        }

        for (uint8_t i = 0; i < schedule.redCount; i++)
        {
            const ScheduleEntry& entry =
                schedule.red[i];

            if (!isDayEnabled(entry, dayOfWeek))
            {
                continue;
            }

            const uint8_t brightness =
                calculateBrightness(
                    entry,
                    currentMinute
                );

            state.lamps[lampIndex].red = brightness;
        }

        for (uint8_t i = 0; i < schedule.blueCount; i++)
        {
            const ScheduleEntry& entry =
                schedule.blue[i];

            if (!isDayEnabled(entry, dayOfWeek))
            {
                continue;
            }

            const uint8_t brightness =
                calculateBrightness(
                    entry,
                    currentMinute
                );

            state.lamps[lampIndex].blue = brightness;
        }
    }

    return state;
}

uint8_t Schedule::getLampIndex(Lamp lamp) const
{
    return static_cast<uint8_t>(lamp);
}

uint8_t Schedule::calculateBrightness(
    const ScheduleEntry& entry,
    uint16_t currentMinute
) const
{
    if (!isEntryActive(entry, currentMinute))
    {
        return 0;
    }

    if (entry.brightness == 0)
    {
        return 0;
    }

    // No fade.
    if (
        entry.fadeInMinutes == 0 &&
        entry.fadeOutMinutes == 0
    )
    {
        if (currentMinute >= entry.endMinute)
        {
            return 0;
        }

        return entry.brightness;
    }

    // Fade-in.
    if (
        entry.fadeInMinutes > 0 &&
        currentMinute <
            entry.startMinute + entry.fadeInMinutes
    )
    {
        const uint16_t elapsed =
            currentMinute - entry.startMinute;

        const float progress =
            static_cast<float>(elapsed) /
            static_cast<float>(entry.fadeInMinutes);

        return static_cast<uint8_t>(
            entry.brightness * progress
        );
    }

    // Fade-out.
    if (
        entry.fadeOutMinutes > 0 &&
        currentMinute >
            entry.endMinute - entry.fadeOutMinutes
    )
    {
        const uint16_t remaining =
            entry.endMinute - currentMinute;

        const float progress =
            static_cast<float>(remaining) /
            static_cast<float>(entry.fadeOutMinutes);

        return static_cast<uint8_t>(
            entry.brightness * progress
        );
    }

    return entry.brightness;
}

bool Schedule::isDayEnabled(
    const ScheduleEntry& entry,
    uint8_t dayOfWeek
) const
{
    return (entry.days & (1 << dayOfWeek)) != 0;
}

bool Schedule::isEntryActive(
    const ScheduleEntry& entry,
    uint16_t currentMinute
) const
{
    return
        currentMinute >= entry.startMinute &&
        currentMinute <= entry.endMinute;
}

bool Schedule::hasOverlap(
    const ScheduleEntry& first,
    const ScheduleEntry& second
) const
{
    // Intervals are [start, end).
    //
    // Therefore:
    //
    // 07:00-10:00
    // 10:00-12:00
    //
    // do NOT overlap.

    return
        first.startMinute < second.endMinute &&
        second.startMinute < first.endMinute;
}

void Schedule::setTimezoneOffsetMinutes(
    int16_t offsetMinutes
)
{
    timezoneOffsetMinutes = offsetMinutes;
}

int16_t Schedule::getTimezoneOffsetMinutes() const
{
    return timezoneOffsetMinutes;
}