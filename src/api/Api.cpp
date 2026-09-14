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
    String json = "{";
    json += "\"brightness\":";
    json += lighting.getBrightness();
    json += "}";

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

    lighting.setBrightness(value);

    String json = "{";
    json += "\"brightness\":";
    json += lighting.getBrightness();
    json += "}";

    server.send(
        200,
        "application/json",
        json
    );
}