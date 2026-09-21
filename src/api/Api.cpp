#include "Api.h"

#include <Arduino.h>
#include <ArduinoJson.h>

Api::Api(
    Lighting& lighting,
    SystemInfo& systemInfo,
    Sensors& sensors,
    Rtc& rtc
)
    : lighting(lighting),
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
}

bool Api::handleRequest(WebServer& server)
{
    const String uri = server.uri();

    if (!uri.startsWith("/api/lamp/"))
    {
        return false;
    }

    // /api/lamp/1/red
    String path = uri.substring(
        String("/api/lamp/").length()
    );

    const int separator = path.indexOf('/');

    if (separator < 0)
    {
        server.send(
            400,
            "application/json",
            R"({"error":"invalid lamp endpoint"})"
        );

        return true;
    }

    const String lampValue =
        path.substring(0, separator);

    const String channelValue =
        path.substring(separator + 1);

    Lamp lamp;
    Channel channel;

    if (!parseLamp(lampValue, lamp))
    {
        server.send(
            400,
            "application/json",
            R"({"error":"invalid lamp"})"
        );

        return true;
    }

    if (!parseChannel(channelValue, channel))
    {
        server.send(
            400,
            "application/json",
            R"({"error":"invalid channel"})"
        );

        return true;
    }

    if (server.method() != HTTP_POST)
    {
        server.send(
            405,
            "application/json",
            R"({"error":"method not allowed"})"
        );

        return true;
    }

    handleSetBrightness(server, lamp, channel);

    return true;
}

void Api::handleState(WebServer& server)
{
    sendState(server);
}

void Api::sendState(WebServer& server)
{
    const LightingState state = lighting.getState();

    JsonDocument doc;

    JsonArray lamps = doc["lamps"].to<JsonArray>();

    for (uint8_t i = 0; i < Lighting::LAMP_COUNT; i++)
    {
        JsonObject lamp = lamps.add<JsonObject>();

        lamp["id"] = i + 1;
        lamp["red"] = state.lamps[i].red;
        lamp["blue"] = state.lamps[i].blue;
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

    lighting.setBrightness(
        lamp,
        channel,
        static_cast<uint8_t>(value)
    );

    sendState(server);
}

bool Api::parseLamp(
    const String& value,
    Lamp& lamp
)
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
)
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