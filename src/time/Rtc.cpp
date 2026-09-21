#include "Rtc.h"

bool Rtc::begin()
{
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!rtc.begin())
    {
        Serial.println("RTC initialization failed");
        return false;
    }
    Serial.println("RTC initialized");
    if (rtc.lostPower())
    {
        Serial.println("RTC lost power");
    }

    return true;
}

DateTime Rtc::getDateTime()
{
    return rtc.now();
}

void Rtc::setDateTime(const DateTime& dateTime)
{
    rtc.adjust(dateTime);
}

bool Rtc::lostPower()
{
    return rtc.lostPower();
}