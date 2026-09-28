#include "LightingController.h"

LightingController::LightingController(
    Lighting& lighting,
    Schedule& schedule,
    Rtc& rtc
)
    : lighting(lighting),
      schedule(schedule),
      rtc(rtc)
{
}

void LightingController::begin()
{
    effectiveState = lighting.getState();

    updateNow();
}

void LightingController::update(
    const DateTime& now
)
{
    update(now, false);
}

void LightingController::updateNow()
{
    const DateTime now = rtc.getDateTime();

    update(now, true);
}

void LightingController::update(
    const DateTime& now,
    bool force
)
{
    const int32_t currentScheduleMinute =
        static_cast<int32_t>(
            now.unixtime() / 60
        );

    if (
        !force &&
        currentScheduleMinute == lastScheduleMinute
    )
    {
        return;
    }

    lastScheduleMinute =
        currentScheduleMinute;

    const ScheduleState scheduleState =
        schedule.getState(now);

    LightingState target =
        lighting.getManualState();

    for (
        uint8_t lampIndex = 0;
        lampIndex < Lighting::LAMP_COUNT;
        lampIndex++
    )
    {
        const Lamp lamp =
            static_cast<Lamp>(lampIndex);

        if (schedule.isEnabled(lamp))
        {
            target.lamps[lampIndex] =
                scheduleState.lamps[lampIndex];
        }
    }

    applyState(target);
}

bool LightingController::setManualBrightness(
    Lamp lamp,
    Channel channel,
    uint8_t percent
)
{
    if (schedule.isEnabled(lamp))
    {
        return false;
    }

    if (!lighting.setManualBrightness(
            lamp,
            channel,
            percent))
    {
        return false;
    }

    updateNow();

    return true;
}

ScheduleError LightingController::addScheduleEntry(
    Lamp lamp,
    Channel channel,
    const ScheduleEntry& entry
)
{
    const ScheduleError error =
        schedule.addEntry(
            lamp,
            channel,
            entry
        );

    if (error != ScheduleError::None)
        return error;

    if (!schedule.save())
    {
        Serial.println(
            "Failed to save schedule after add"
        );
    }

    updateNow();

    return ScheduleError::None;
}

ScheduleError LightingController::updateScheduleEntry(
    Lamp lamp,
    Channel channel,
    uint8_t index,
    const ScheduleEntry& entry
)
{
    const ScheduleError error =
        schedule.updateEntry(
            lamp,
            channel,
            index,
            entry
        );

    if (error != ScheduleError::None)
        return error;

    if (!schedule.save())
    {
        Serial.println(
            "Failed to save schedule after update"
        );
    }

    updateNow();

    return ScheduleError::None;
}

bool LightingController::deleteScheduleEntry(
    Lamp lamp,
    Channel channel,
    uint8_t index
)
{
    if (!schedule.removeEntry(lamp, channel, index))
        return false;

    if (!schedule.save())
    {
        Serial.println(
            "Failed to save schedule after delete"
        );
    }

    updateNow();

    return true;
}

void LightingController::setScheduleEnabled(
    Lamp lamp,
    bool enabled
)
{
    schedule.setEnabled(
        lamp,
        enabled
    );

    if (!schedule.save())
    {
        Serial.println(
            "Failed to save schedule after enable change"
        );
    }

    updateNow();
}

bool LightingController::isScheduleEnabled(
    Lamp lamp
) const
{
    return schedule.isEnabled(lamp);
}

bool LightingController::setTimezoneOffsetMinutes(
    int16_t offsetMinutes
)
{
    schedule.setTimezoneOffsetMinutes(
        offsetMinutes
    );

    if (!schedule.save())
    {
        Serial.println(
            "Failed to save timezone"
        );

        return false;
    }

    updateNow();

    return true;
}

int16_t LightingController::getTimezoneOffsetMinutes() const
{
    return schedule.getTimezoneOffsetMinutes();
}

LightingState LightingController::getEffectiveState() const
{
    return effectiveState;
}

LightingState LightingController::getManualState() const
{
    return lighting.getManualState();
}

const Schedule& LightingController::getSchedule() const
{
    return schedule;
}

void LightingController::applyState(
    const LightingState& state
)
{
    if (state == effectiveState)
    {
        return;
    }

    lighting.setState(state);

    effectiveState = state;
}