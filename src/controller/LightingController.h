#pragma once

#include "../lighting/Lighting.h"
#include "../schedule/Schedule.h"
#include "../time/Rtc.h"

class LightingController
{
public:
    LightingController(
        Lighting& lighting,
        Schedule& schedule,
        Rtc& rtc
    );

    void begin();

    void update();

    bool setManualBrightness(
        Lamp lamp,
        Channel channel,
        uint8_t percent
    );

    ScheduleError addScheduleEntry(
        Lamp lamp,
        Channel channel,
        const ScheduleEntry& entry
    );

    ScheduleError updateScheduleEntry(
        Lamp lamp,
        Channel channel,
        uint8_t index,
        const ScheduleEntry& entry
    );

    bool deleteScheduleEntry(
        Lamp lamp,
        Channel channel,
        uint8_t index
    );

    void setScheduleEnabled(
        Lamp lamp,
        bool enabled
    );

    bool isScheduleEnabled(
        Lamp lamp
    ) const;

    LightingState getEffectiveState() const;

    LightingState getManualState() const;

    const Schedule& getSchedule() const;

private:
    Lighting& lighting;
    Schedule& schedule;
    Rtc& rtc;

    LightingState effectiveState{};

    // Минутный timestamp последнего расчёта расписания
    // -1 означает "ещё не инициализирован"
    int32_t lastScheduleMinute = -1;

    void update(bool force);

    void applyState(
        const LightingState& state
    );
};