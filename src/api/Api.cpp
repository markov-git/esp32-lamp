#include "Api.h"

#include <Arduino.h>

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
    const LightingState state = lighting.getState();
    String json = R"({"lamps":[)";

    for (uint8_t lampIndex = 0; lampIndex < Lighting::LAMP_COUNT; lampIndex++)
    {
        if (lampIndex > 0)
        {
            json += ",";
        }

        json += "{";
        json += "\"id\":";
        json += lampIndex + 1;

        json += ",\"red\":";
        json += state.lamps[lampIndex].red;

        json += ",\"blue\":";
        json += state.lamps[lampIndex].blue;

        json += "}";
    }

    json += "]}";

    server.send(
        200,
        "application/json",
        json
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

    String json = "{";
    json += "\"brightness\":";
    json += lighting.getBrightness(
        lamp,
        channel
    );
    json += "}";

    server.send(
        200,
        "application/json",
        json
    );
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