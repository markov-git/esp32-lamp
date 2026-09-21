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
    static constexpr uint8_t SDA_PIN = 21;
    static constexpr uint8_t SCL_PIN = 22;

    RTC_DS3231 rtc;
};