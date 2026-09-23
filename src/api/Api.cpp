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

        soil["id"] = i;
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