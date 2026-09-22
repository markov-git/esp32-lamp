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

    void applyState(
        const LightingState& state
    );
};