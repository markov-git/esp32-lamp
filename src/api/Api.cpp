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

bool Api::handleRequest(WebServer& server)
{
    const String path = server.uri();

    if (!path.startsWith("/api/"))
    {
        return false;
    }

    if (
        path == "/api/state" &&
        server.method() == HTTP_GET
    )
    {
        handleState(server);
        return true;
    }

    if (
        path == "/api/system" &&
        server.method() == HTTP_GET
    )
    {
        handleSystem(server);
        return true;
    }

    if (
        path == "/api/sensors" &&
        server.method() == HTTP_GET
    )
    {
        handleSensors(server);
        return true;
    }

    if (
        path == "/api/time" &&
        server.method() == HTTP_GET
    )
    {
        handleTime(server);
        return true;
    }

    if (
        path == "/api/time" &&
        server.method() == HTTP_POST
    )
    {
        handleSetTime(server);
        return true;
    }

    Lamp lamp;
    Channel channel;

    if (
        server.method() == HTTP_POST &&
        parseLampChannel(path, lamp, channel)
    )
    {
        handleSetBrightness(
            server,
            lamp,
            channel
        );

        return true;
    }

    if (
        server.method() == HTTP_POST &&
        parseLampSchedule(path, lamp)
    )
    {
        handleSetScheduleEnabled(
            server,
            lamp
        );

        return true;
    }

    return false;
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

void Api::handleSetBrightness(
    WebServer& server,
    Lamp lamp,
    Channel channel
)
{
    if (!server.hasArg("value"))
    {
        server.send(
            400,
            "application/json",
            R"({"error":"missing value"})"
        );

        return;
    }

    const int value =
        server.arg("value").toInt();

    if (value < 0 || value > 100)
    {
        server.send(
            400,
            "application/json",
            R"({"error":"value must be between 0 and 100"})"
        );

        return;
    }

    bool setResult = lightingController.setManualBrightness(
        lamp,
        channel,
        static_cast<uint8_t>(value)
    );
    if (!setResult)
    {
        server.send(
            409,
            "application/json",
            "{\"error\":\"schedule_enabled\"}"
        );

        return;
    }
    

    sendState(server);
}

void Api::handleSetScheduleEnabled(
    WebServer& server,
    Lamp lamp
)
{
    if (!server.hasArg("enabled"))
    {
        sendJsonError(
            server,
            400,
            "missing_enabled"
        );

        return;
    }

    const String value =
        server.arg("enabled");

    bool enabled;

    if (value == "true" || value == "1")
    {
        enabled = true;
    }
    else if (
        value == "false" ||
        value == "0"
    )
    {
        enabled = false;
    }
    else
    {
        sendJsonError(
            server,
            400,
            "invalid_enabled"
        );

        return;
    }

    lightingController.setScheduleEnabled(
        lamp,
        enabled
    );

    sendState(server);
}

bool Api::parseLamp(
    const String& value,
    Lamp& lamp
) const
{
    if (value == "1")
    {
        lamp = Lamp::Lamp1;
        return true;
    }

    if (value == "2")
    {
        lamp = Lamp::Lamp2;
        return true;
    }

    if (value == "3")
    {
        lamp = Lamp::Lamp3;
        return true;
    }

    return false;
}

bool Api::parseChannel(
    const String& value,
    Channel& channel
) const
{
    if (value == "red")
    {
        channel = Channel::Red;
        return true;
    }

    if (value == "blue")
    {
        channel = Channel::Blue;
        return true;
    }

    return false;
}

bool Api::parseLampChannel(
    const String& path,
    Lamp& lamp,
    Channel& channel
) const
{
    if (!path.startsWith("/api/lamp/"))
    {
        return false;
    }

    const String prefix = "/api/lamp/";

    String remainder =
        path.substring(prefix.length());

    const int slashIndex =
        remainder.indexOf('/');

    if (slashIndex <= 0)
    {
        return false;
    }

    const String lampPart =
        remainder.substring(
            0,
            slashIndex
        );

    const String channelPart =
        remainder.substring(
            slashIndex + 1
        );

    if (!parseLamp(
            lampPart,
            lamp))
    {
        return false;
    }

    return parseChannel(
        channelPart,
        channel
    );
}

bool Api::parseLampSchedule(
    const String& path,
    Lamp& lamp
) const
{
    const String prefix =
        "/api/lamp/";

    if (!path.startsWith(prefix))
    {
        return false;
    }

    String remainder =
        path.substring(prefix.length());

    const String suffix =
        "/schedule";

    if (!remainder.endsWith(suffix))
    {
        return false;
    }

    remainder =
        remainder.substring(
            0,
            remainder.length() -
                suffix.length()
        );

    return parseLamp(
        remainder,
        lamp
    );
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
    if (!server.hasArg("plain"))
    {
        server.send(
            400,
            "application/json",
            "{\"error\":\"Request body is required\"}"
        );

        return;
    }

    JsonDocument doc;

    const DeserializationError error =
        deserializeJson(doc, server.arg("plain"));

    if (error)
    {
        server.send(
            400,
            "application/json",
            "{\"error\":\"Invalid JSON\"}"
        );

        return;
    }

    if (!doc["unix"].is<uint64_t>())
    {
        server.send(
            400,
            "application/json",
            "{\"error\":\"unix is required\"}"
        );

        return;
    }

    const uint64_t timestamp = doc["unix"];

    rtc.setDateTime(DateTime(timestamp));

    sendTime(server);
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