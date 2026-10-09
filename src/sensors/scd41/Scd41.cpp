#include "Scd41.h"

bool Scd41::begin() {
    sensor.begin(Wire, I2C_ADDRESS);

    // На случай, если периодическое измерение уже запущено.
    uint16_t error = sensor.stopPeriodicMeasurement();

    if (error != 0) {
        Serial.print("SCD41: stopPeriodicMeasurement failed: ");
        Serial.println(error);
    }

    error = sensor.startPeriodicMeasurement();

    if (error != 0) {
        Serial.print("SCD41: startPeriodicMeasurement failed: ");
        Serial.println(error);
        started = false;
        return false;
    }

    started = true;
    lastCheckMs = millis();

    Serial.println("SCD41: periodic measurement started");
    return true;
}

void Scd41::update() {
    if (!started) {
        return;
    }

    const uint32_t now = millis();

    if (now - lastCheckMs < CHECK_INTERVAL_MS) {
        return;
    }

    lastCheckMs = now;

    bool dataReady = false;
    uint16_t error = sensor.getDataReadyStatus(dataReady);

    if (error != 0) {
        Serial.print("SCD41: getDataReadyStatus failed: ");
        Serial.println(error);
        return;
    }

    if (!dataReady) {
        return;
    }

    uint16_t co2 = 0;
    float temperature = NAN;
    float humidity = NAN;

    error = sensor.readMeasurement(co2, temperature, humidity);

    if (error != 0) {
        Serial.print("SCD41: readMeasurement failed: ");
        Serial.println(error);
        return;
    }

    // CO2 = 0 означает, что измерение ещё не готово
    // или результат невалиден.
    if (co2 == 0 ||
        !isfinite(temperature) ||
        !isfinite(humidity)) {
        Serial.println("SCD41: invalid measurement");
        return;
    }

    state.co2Ppm = co2;
    state.temperature = temperature;
    state.humidity = humidity;
    state.available = true;
}

const Scd41State& Scd41::getState() {
    update();
    return state;
}