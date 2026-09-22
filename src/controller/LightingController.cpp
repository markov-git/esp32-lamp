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

    update();
}

void LightingController::update()
{
    const DateTime now =
        rtc.getDateTime();

    const ScheduleState scheduleState =
        schedule.getState(now);

    LightingState target =
        lighting.getManualState();

    for (uint8_t lampIndex = 0;
         lampIndex < Lighting::LAMP_COUNT;
         lampIndex++)
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

    update();

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

    update();
}

bool LightingController::isScheduleEnabled(
    Lamp lamp
) const
{
    return schedule.isEnabled(lamp);
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