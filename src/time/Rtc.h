#pragma once

#include <Arduino.h>
#include <RTClib.h>

class Rtc
{
public:
    bool begin();

    DateTime getDateTime();

    void setDateTime(const DateTime& dateTime);

    bool lostPower();

private:
    RTC_DS3231 rtc;
};