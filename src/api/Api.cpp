#include "Api.h"

Api::Api(
    LightingController& lightingController,
    SystemInfo& systemInfo,
    Sensors& sensors,
    Rtc& rtc
)
    : lightingController(lightingController),
    systemInfo(systemInfo),
    sensors(sensors),
    rtc(rtc)
{
}

void Api::registerRoutes(WebServer& server)
{
    server.on(
        "/api/state",
        HTTP_GET,
        [this, &server]()
        {
            handleState(server);
        }
    );
    server.on(
        "/api/system",
        HTTP_GET,
        [&]() {
            handleSystem(server);
        }
    );
    server.on(
        "/api/sensors",
        HTTP_GET,
        [&]() {
            handleSensors(server);
        }
    );
    server.on(
        "/api/time",
        HTTP_GET,
        [&]() {
            handleTime(server);
        }
    );
    server.on(
        "/api/time",
        HTTP_POST,
        [&]() {
            handleSetTime(server);
        }
    );

    server.on(
        "/api/lighting/manual",
        HTTP_POST,
        [this, &server]()
        {
            handleSetManualBrightness(server);
        }
    );

    server.on(
        "/api/lighting/schedule",
        HTTP_POST,
        [this, &server]()
        {
            handleSetScheduleEnabled(server);
        }
    );

    server.on(
        "/api/schedules",
        HTTP_GET,
        [this, &server]()
        {
            handleSchedules(server);
        }
    );

    server.on(
        "/api/schedules/add",
        HTTP_POST,
        [this, &server]()
        {
            handleAddSchedule(server);
        }
    );

    server.on(
        "/api/schedules/delete",
        HTTP_POST,
        [this, &server]()
        {
            handleDeleteSchedule(server);
        }
    );
}

void Api::handleState(WebServer& server)
{
    sendState(server);
}

void Api::sendState(WebServer& server)
{
    const LightingState effectiveState = lightingController.getEffectiveState();
    const LightingState manualState = lightingController.getManualState();

    JsonDocument doc;

    JsonArray lamps = doc["lamps"].to<JsonArray>();

    for (uint8_t i = 0; i < Lighting::LAMP_COUNT; i++)
    {
        JsonObject lamp = lamps.add<JsonObject>();

        lamp["id"] = i + 1;

        JsonObject current =
            lamp["current"].to<JsonObject>();

        current["red"] = effectiveState.lamps[i].red;
        current["blue"] = effectiveState.lamps[i].blue;

        JsonObject manual =
            lamp["manual"].to<JsonObject>();

        manual["red"] =
            manualState.lamps[i].red;

        manual["blue"] =
            manualState.lamps[i].blue;

        lamp["scheduleEnabled"] =
            lightingController.isScheduleEnabled(
                static_cast<Lamp>(i)
            );
    }

    // Time
    JsonObject time = doc["time"].to<JsonObject>();

    time["unix"] = rtc.getDateTime().unixtime();
    time["lostPower"] = rtc.lostPower();

    String output;

    serializeJson(doc, output);

    server.send(
        200,
        "application/json",
        output
    );
}

void Api::handleSystem(WebServer& server)
{
    sendSystem(server);
}
void Api::handleSensors(WebServer& server)
{
    sendSensors(server);
}

void Api::handleSetScheduleEnabled(
    WebServer& server
)
{
    JsonDocument doc;

    if (!parseJsonBody(server, doc))
    {
        return;
    }

    Lamp lamp;

    if (!parseLamp(doc["lamp"], lamp))
    {
        sendJsonError(
            server,
            400,
            "invalid_lamp"
        );

        return;
    }

    if (!doc["enabled"].is<bool>())
    {
        sendJsonError(
            server,
            400,
            "missing_enabled"
        );

        return;
    }

    const bool enabled =
        doc["enabled"].as<bool>();

    lightingController.setScheduleEnabled(
        lamp,
        enabled
    );

    sendState(server);
}

bool Api::parseLamp(
    JsonVariantConst value,
    Lamp& lamp
) const
{
    if (!value.is<uint8_t>())
    {
        return false;
    }

    const uint8_t number =
        value.as<uint8_t>();

    switch (number)
    {
        case 1:
            lamp = Lamp::Lamp1;
            return true;

        case 2:
            lamp = Lamp::Lamp2;
            return true;

        case 3:
            lamp = Lamp::Lamp3;
            return true;

        default:
            return false;
    }
}

bool Api::parseChannel(
    JsonVariantConst value,
    Channel& channel
) const
{
    if (!value.is<const char*>())
    {
        return false;
    }

    const char* valueString =
        value.as<const char*>();

    if (strcmp(valueString, "red") == 0)
    {
        channel = Channel::Red;
        return true;
    }

    if (strcmp(valueString, "blue") == 0)
    {
        channel = Channel::Blue;
        return true;
    }

    return false;
}

void Api::sendSystem(WebServer& server)
{
    const SystemInfoState state = systemInfo.getState();

    JsonDocument doc;

    doc["chipModel"] = state.chipModel;
    doc["chipRevision"] = state.chipRevision;
    doc["cpuCores"] = state.cpuCores;
    doc["cpuFrequencyMhz"] = state.cpuFrequencyMhz;

    doc["uptimeSeconds"] = state.uptimeSeconds;
    doc["freeHeap"] = state.freeHeap;
    doc["totalHeap"] = state.totalHeap;
    doc["minimumFreeHeap"] = state.minimumFreeHeap;

    doc["flashSize"] = state.flashSize;
    doc["sketchSize"] = state.sketchSize;
    doc["freeSketchSpace"] = state.freeSketchSpace;

    doc["filesystemTotal"] = state.filesystemTotal;
    doc["filesystemUsed"] = state.filesystemUsed;
    doc["filesystemFree"] = state.filesystemFree;

    doc["ip"] = state.ip;
    doc["gateway"] = state.gateway;
    doc["subnet"] = state.subnet;
    doc["mac"] = state.mac;
    doc["wifiRssi"] = state.wifiRssi;

    doc["chipTemperature"] = state.chipTemperature;

    String json;
    serializeJson(doc, json);

    server.send(
        200,
        "application/json",
        json
    );
}

void Api::sendSensors(WebServer& server)
{
    const SensorsState state = sensors.getState();

    JsonDocument doc;

    JsonObject bme280 = doc["bme280"].to<JsonObject>();

    bme280["temperature"] = state.bme280.temperature;
    bme280["humidity"] = state.bme280.humidity;
    bme280["pressure"] = state.bme280.pressure;

    JsonArray soilMoisture = doc["soilMoisture"].to<JsonArray>();
    for (uint8_t i = 0; i < SoilMoisture::SENSOR_COUNT; i++)
    {
        JsonObject soil = soilMoisture.add<JsonObject>();

        soil["id"] = i + 1;
        soil["raw"] = state.soilMoisture.raw[i];
        soil["percent"] = state.soilMoisture.percent[i];
    }

    String json;
    serializeJson(doc, json);

    server.send(
        200,
        "application/json",
        json
    );
}
void Api::handleTime(WebServer& server)
{
    sendTime(server);
}

void Api::sendTime(WebServer& server)
{
    const DateTime now = rtc.getDateTime();

    JsonDocument doc;

    doc["unix"] = now.unixtime();
    doc["lostPower"] = rtc.lostPower();

    String json;
    serializeJson(doc, json);

    server.send(
        200,
        "application/json",
        json
    );
}

void Api::handleSetTime(WebServer& server)
{
    JsonDocument doc;

    if (!parseJsonBody(server, doc))
    {
        return;
    }

    if (!doc["unix"].is<uint32_t>())
    {
        sendJsonError(
            server,
            400,
            "missing_unix"
        );

        return;
    }

    const uint32_t unixTime =
        doc["unix"].as<uint32_t>();

    rtc.setDateTime(
        DateTime(unixTime)
    );

    sendTime(server);
}

void Api::handleSetManualBrightness(
    WebServer& server
)
{
    JsonDocument doc;

    if (!parseJsonBody(server, doc))
    {
        return;
    }

    Lamp lamp;
    Channel channel;

    if (!parseLamp(doc["lamp"], lamp))
    {
        sendJsonError(
            server,
            400,
            "invalid_lamp"
        );

        return;
    }

    if (!parseChannel(doc["channel"], channel))
    {
        sendJsonError(
            server,
            400,
            "invalid_channel"
        );

        return;
    }

    if (!doc["brightness"].is<uint8_t>())
    {
        sendJsonError(
            server,
            400,
            "missing_brightness"
        );

        return;
    }

    const uint8_t brightness =
        doc["brightness"].as<uint8_t>();

    if (brightness > 100)
    {
        sendJsonError(
            server,
            400,
            "invalid_brightness"
        );

        return;
    }

    if (!lightingController.setManualBrightness(
            lamp,
            channel,
            brightness))
    {
        sendJsonError(
            server,
            409,
            "schedule_enabled"
        );

        return;
    }

    sendState(server);
}

void Api::handleSchedules(WebServer& server)
{
    JsonDocument doc;

    JsonArray lamps = doc["lamps"].to<JsonArray>();

    for (uint8_t i = 0; i < Lighting::LAMP_COUNT; i++)
    {
        const Lamp lamp =
            static_cast<Lamp>(i);

        const LampSchedule& schedule =
            lightingController.getSchedule().getSchedule(lamp);

        JsonObject lampObject =
            lamps.add<JsonObject>();

        lampObject["id"] = i + 1;

        lampObject["enabled"] =
            schedule.enabled;

        JsonArray red =
            lampObject["red"].to<JsonArray>();

        for (uint8_t j = 0; j < schedule.redCount; j++)
        {
            JsonObject entry =
                red.add<JsonObject>();

            entry["days"] =
                schedule.red[j].days;

            entry["start"] =
                schedule.red[j].startMinute;

            entry["end"] =
                schedule.red[j].endMinute;

            entry["brightness"] =
                schedule.red[j].brightness;

            entry["fadeIn"] =
                schedule.red[j].fadeInMinutes;

            entry["fadeOut"] =
                schedule.red[j].fadeOutMinutes;
        }

        JsonArray blue =
            lampObject["blue"].to<JsonArray>();

        for (uint8_t j = 0; j < schedule.blueCount; j++)
        {
            JsonObject entry =
                blue.add<JsonObject>();

            entry["days"] =
                schedule.blue[j].days;

            entry["start"] =
                schedule.blue[j].startMinute;

            entry["end"] =
                schedule.blue[j].endMinute;

            entry["brightness"] =
                schedule.blue[j].brightness;

            entry["fadeIn"] =
                schedule.blue[j].fadeInMinutes;

            entry["fadeOut"] =
                schedule.blue[j].fadeOutMinutes;
        }
    }

    String response;

    serializeJson(doc, response);

    server.send(
        200,
        "application/json",
        response
    );
}

void Api::handleAddSchedule(WebServer& server)
{
    JsonDocument doc;

    if (!parseJsonBody(server, doc))
        return;

    Lamp lamp;
    if (!parseLamp(doc["lamp"], lamp))
    {
        sendJsonError(
            server,
            400,
            "invalid_lamp"
        );

        return;
    }

    Channel channel;
    if (!parseChannel(doc["channel"], channel))
    {
        sendJsonError(
            server,
            400,
            "invalid_channel"
        );

        return;
    }

    ScheduleEntry entry;

    if (!parseScheduleEntry(doc["entry"], entry))
    {
        sendJsonError(
            server,
            400,
            "invalid_entry"
        );

        return;
    }

    const ScheduleError error =
        lightingController.addScheduleEntry(
            lamp,
            channel,
            entry
        );

    if (error != ScheduleError::None)
    {
        switch (error)
        {
            case ScheduleError::InvalidDays:
                sendJsonError(
                    server,
                    400,
                    "invalid_days"
                );
                return;

            case ScheduleError::InvalidTime:
                sendJsonError(
                    server,
                    400,
                    "invalid_time"
                );
                return;

            case ScheduleError::InvalidBrightness:
                sendJsonError(
                    server,
                    400,
                    "invalid_brightness"
                );
                return;

            case ScheduleError::InvalidFade:
                sendJsonError(
                    server,
                    400,
                    "invalid_fade"
                );
                return;

            case ScheduleError::MaxEntries:
                sendJsonError(
                    server,
                    409,
                    "max_entries"
                );
                return;

            case ScheduleError::Overlap:
                sendJsonError(
                    server,
                    409,
                    "overlap"
                );
                return;

            case ScheduleError::None:
                break;
        }
    }

    handleSchedules(server);
}

void Api::handleDeleteSchedule(WebServer& server)
{
    JsonDocument doc;

    if (!parseJsonBody(server, doc))
        return;

    Lamp lamp;

    if (!parseLamp(doc["lamp"], lamp))
    {
        sendJsonError(
            server,
            400,
            "invalid_lamp"
        );

        return;
    }

    Channel channel;

    if (!parseChannel(doc["channel"], channel))
    {
        sendJsonError(
            server,
            400,
            "invalid_channel"
        );

        return;
    }

    if (!doc["index"].is<int>())
    {
        sendJsonError(
            server,
            400,
            "invalid_index"
        );

        return;
    }

    const int index =
        doc["index"].as<int>();

    if (index < 0 || index >= Schedule::MAX_ENTRIES_PER_CHANNEL)
    {
        sendJsonError(
            server,
            400,
            "invalid_index"
        );

        return;
    }

    if (!lightingController.deleteScheduleEntry(
        lamp,
        channel,
        static_cast<uint8_t>(index)
    ))
    {
        sendJsonError(
            server,
            404,
            "entry_not_found"
        );

        return;
    }

    handleSchedules(server);
}

void Api::sendJsonError(
    WebServer& server,
    int statusCode,
    const char* error
)
{
    JsonDocument doc;

    doc["error"] = error;

    String response;

    serializeJson(doc, response);

    server.send(
        statusCode,
        "application/json",
        response
    );
}

bool Api::parseJsonBody(
    WebServer& server,
    JsonDocument& doc
)
{
    if (!server.hasArg("plain"))
    {
        sendJsonError(
            server,
            400,
            "missing_body"
        );

        return false;
    }

    const DeserializationError error =
        deserializeJson(
            doc,
            server.arg("plain")
        );

    if (error)
    {
        sendJsonError(
            server,
            400,
            "invalid_json"
        );

        return false;
    }

    return true;
}

bool Api::parseScheduleEntry(
    JsonVariantConst value,
    ScheduleEntry& entry
)
{
    if (!value.is<JsonObjectConst>())
        return false;

    JsonObjectConst object =
        value.as<JsonObjectConst>();

    if (
        !object["days"].is<int>() ||
        !object["start"].is<int>() ||
        !object["end"].is<int>() ||
        !object["brightness"].is<int>() ||
        !object["fadeIn"].is<int>() ||
        !object["fadeOut"].is<int>()
    )
    {
        return false;
    }

    const int days =
        object["days"].as<int>();

    const int start =
        object["start"].as<int>();

    const int end =
        object["end"].as<int>();

    const int brightness =
        object["brightness"].as<int>();

    const int fadeIn =
        object["fadeIn"].as<int>();

    const int fadeOut =
        object["fadeOut"].as<int>();

    if (
        days < 0 || days > 127 ||
        start < 0 || start > 1440 ||
        end < 0 || end > 1440 ||
        brightness < 0 || brightness > 100 ||
        fadeIn < 0 || fadeIn > 1440 ||
        fadeOut < 0 || fadeOut > 1440
    )
    {
        return false;
    }

    entry.days =
        static_cast<uint8_t>(days);

    entry.startMinute =
        static_cast<uint16_t>(start);

    entry.endMinute =
        static_cast<uint16_t>(end);

    entry.brightness =
        static_cast<uint8_t>(brightness);

    entry.fadeInMinutes =
        static_cast<uint16_t>(fadeIn);

    entry.fadeOutMinutes =
        static_cast<uint16_t>(fadeOut);

    return true;
}