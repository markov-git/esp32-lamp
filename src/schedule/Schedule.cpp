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

bool Schedule::addEntry(
    Lamp lamp,
    Channel channel,
    const ScheduleEntry& entry
)
{
    const uint8_t lampIndex = getLampIndex(lamp);
    LampSchedule& schedule = schedules[lampIndex];

     // At least one day must be selected.
    if (entry.days == 0)
    {
        return false;
    }

    // Only 7 days are valid.
    if (entry.days & ~DAYS_MASK)
    {
        return false;
    }

    // Interval must be inside one day.
    if (entry.startMinute >= MINUTES_PER_DAY)
    {
        return false;
    }

    if (entry.endMinute > MINUTES_PER_DAY)
    {
        return false;
    }

    // Midnight crossing is not supported.
    if (entry.startMinute >= entry.endMinute)
    {
        return false;
    }

    // Brightness must be 0..100%.
    if (entry.brightness > 100)
    {
        return false;
    }

    const uint16_t duration =
        entry.endMinute - entry.startMinute;

    // Fade cannot be longer than the interval.
    if (entry.fadeInMinutes > duration)
    {
        return false;
    }

    if (entry.fadeOutMinutes > duration)
    {
        return false;
    }

    if (channel == Channel::Red)
    {
        if (schedule.redCount >= MAX_ENTRIES_PER_CHANNEL)
        {
            return false;
        }

        // Check overlap with existing red entries.
        for (uint8_t i = 0; i < schedule.redCount; i++)
        {
            const ScheduleEntry& existing =
                schedule.red[i];

            // Overlap only matters on common days.
            if ((existing.days & entry.days) == 0)
            {
                continue;
            }

            if (hasOverlap(existing, entry))
            {
                return false;
            }
        }

        schedule.red[schedule.redCount++] = entry;
        return true;
    }

    if (schedule.blueCount >= MAX_ENTRIES_PER_CHANNEL)
    {
        return false;
    }

    // Check overlap with existing blue entries.
    for (uint8_t i = 0; i < schedule.blueCount; i++)
    {
        const ScheduleEntry& existing =
            schedule.blue[i];

        if ((existing.days & entry.days) == 0)
        {
            continue;
        }

        if (hasOverlap(existing, entry))
        {
            return false;
        }
    }

    schedule.blue[schedule.blueCount++] = entry;

    return true;
}

bool Schedule::removeEntry(
    Lamp lamp,
    Channel channel,
    uint8_t index
)
{
    const uint8_t lampIndex = getLampIndex(lamp);
    LampSchedule& schedule = schedules[lampIndex];

    if (channel == Channel::Red)
    {
        if (index >= schedule.redCount)
        {
            return false;
        }

        for (uint8_t i = index;
             i + 1 < schedule.redCount;
             i++)
        {
            schedule.red[i] = schedule.red[i + 1];
        }

        schedule.redCount--;

        return true;
    }

    if (index >= schedule.blueCount)
    {
        return false;
    }

    for (uint8_t i = index;
         i + 1 < schedule.blueCount;
         i++)
    {
        schedule.blue[i] = schedule.blue[i + 1];
    }

    schedule.blueCount--;

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