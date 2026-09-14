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

    server.on(
        "/api/lamp/1/brightness",
        HTTP_POST,
        [this, &server]()
        {
            handleSetBrightness(server);
        }
    );
}

void Api::handleState(WebServer& server)
{
    String json = R"({"lamps":[)";

    for (uint8_t lampIndex = 0; lampIndex < 3; lampIndex++)
    {
        if (lampIndex > 0)
        {
            json += ",";
        }

        const Lamp lamp =
            static_cast<Lamp>(lampIndex);

        json += "{";
        json += "\"id\":";
        json += lampIndex + 1;

        json += ",\"red\":";
        json += lighting.getBrightness(
            lamp,
            Channel::Red
        );

        json += ",\"blue\":";
        json += lighting.getBrightness(
            lamp,
            Channel::Blue
        );

        json += "}";
    }

    json += "]}";

    server.send(
        200,
        "application/json",
        json
    );
}

void Api::handleSetBrightness(WebServer& server)
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

    // Пока временно Lamp1/Red.
    lighting.setBrightness(
        Lamp::Lamp1,
        Channel::Red,
        value
    );

    String json = "{";
    json += "\"red\":";
    json += lighting.getBrightness(
        Lamp::Lamp1,
        Channel::Red
    );
    json += "}";

    server.send(
        200,
        "application/json",
        json
    );
}