#include "WebServerManager.h"

#include <Arduino.h>
#include <LittleFS.h>

#include "../api/Api.h"

WebServerManager::WebServerManager(Api& api)
    : api(api)
{
}

void WebServerManager::begin()
{
    api.registerRoutes(server);

    server.onNotFound(
        [this]()
        {
            handleNotFound();
        }
    );

    server.begin();

    Serial.println("Web server started");
}

void WebServerManager::handleClient()
{
    server.handleClient();
}

void WebServerManager::handleNotFound()
{
    if (server.uri().startsWith("/api/"))
    {
        api.handleRequest(server);
        return;
    }

    handleFile();
}

void WebServerManager::handleFile()
{
    String path = server.uri();

    if (path == "/")
    {
        path = "/index.html";
    }

    if (!LittleFS.exists(path))
    {
        server.send(
            404,
            "text/plain",
            "File not found"
        );

        return;
    }

    File file = LittleFS.open(path, "r");

    if (!file)
    {
        server.send(
            500,
            "text/plain",
            "Failed to open file"
        );

        return;
    }

    server.streamFile(
        file,
        getContentType(path)
    );

    file.close();
}

String WebServerManager::getContentType(const String& path)
{
    if (path.endsWith(".html"))
        return "text/html";

    if (path.endsWith(".css"))
        return "text/css";

    if (path.endsWith(".js"))
        return "application/javascript";

    if (path.endsWith(".json"))
        return "application/json";

    if (path.endsWith(".svg"))
        return "image/svg+xml";

    if (path.endsWith(".png"))
        return "image/png";

    if (path.endsWith(".jpg") || path.endsWith(".jpeg"))
        return "image/jpeg";

    if (path.endsWith(".ico"))
        return "image/x-icon";

    return "text/plain";
}