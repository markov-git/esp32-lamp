#pragma once

#include <Arduino.h>
#include <RTClib.h>

#include "../lighting/Lighting.h"

struct ScheduleEntry
{
    uint8_t days;

    uint16_t startMinute;
    uint16_t endMinute;

    uint8_t brightness;

    uint16_t fadeInMinutes;
    uint16_t fadeOutMinutes;
};

struct LampSchedule
{
    bool enabled;

    ScheduleEntry red[8];
    uint8_t redCount;

    ScheduleEntry blue[8];
    uint8_t blueCount;
};

struct ScheduleState
{
    LampState lamps[Lighting::LAMP_COUNT];
};

class Schedule
{
public:
    static constexpr uint8_t MAX_ENTRIES_PER_CHANNEL = 8;

    Schedule();

    void clear();

    bool addEntry(
        Lamp lamp,
        Channel channel,
        const ScheduleEntry& entry
    );

    bool removeEntry(
        Lamp lamp,
        Channel channel,
        uint8_t index
    );

    void setEnabled(Lamp lamp, bool enabled);

    bool isEnabled(Lamp lamp) const;

    ScheduleState getState(const DateTime& now) const;

private:
    LampSchedule schedules[Lighting::LAMP_COUNT];

    uint8_t getLampIndex(Lamp lamp) const;

    uint8_t calculateBrightness(
        const ScheduleEntry& entry,
        uint16_t currentMinute
    ) const;

    bool isDayEnabled(
        const ScheduleEntry& entry,
        uint8_t dayOfWeek
    ) const;

    bool isEntryActive(
        const ScheduleEntry& entry,
        uint16_t currentMinute
    ) const;

    bool hasOverlap(
        const ScheduleEntry& first,
        const ScheduleEntry& second
    ) const;
};