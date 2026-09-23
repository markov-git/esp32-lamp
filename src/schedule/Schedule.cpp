#include "Schedule.h"

namespace
{
    constexpr uint8_t DAYS_MASK = 0b01111111;
    constexpr uint16_t MINUTES_PER_DAY = 24 * 60;
}

Schedule::Schedule()
{
    clear();
}

void Schedule::clear()
{
    for (uint8_t i = 0; i < Lighting::LAMP_COUNT; i++)
    {
        schedules[i].enabled = false;

        schedules[i].redCount = 0;
        schedules[i].blueCount = 0;
    }
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

    const uint16_t currentMinute =
        now.hour() * 60 + now.minute();

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