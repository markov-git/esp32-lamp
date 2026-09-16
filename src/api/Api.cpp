#include "Api.h"

#include <Arduino.h>
#include <ArduinoJson.h>

Api::Api(Lighting& lighting)
    : lighting(lighting)
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

    String output;

    serializeJson(doc, output);

    server.send(
        200,
        "application/json",
        output
    );
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