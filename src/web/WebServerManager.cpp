#include "WebServerManager.h"

#include <LittleFS.h>

WebServerManager::WebServerManager(WebServer& server)
    : server(server)
{
}

void WebServerManager::begin()
{
    registerRoutes();

    Serial.println("Web server started");
}

void WebServerManager::registerRoutes()
{
    server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        }
    );

    server.onNotFound(
        [this]()
        {
            handleNotFound();
        }
    );
}

void WebServerManager::handleRoot()
{
    File file = LittleFS.open("/index.html", "r");

    if (!file)
    {
        server.send(
            500,
            "text/plain",
            "index.html not found"
        );

        return;
    }

    server.streamFile(file, "text/html");
    file.close();
}

void WebServerManager::handleNotFound()
{
    handleStaticFile();
}

void WebServerManager::handleStaticFile()
{
    String path = server.uri();
    String gzPath = path + ".gz";

    // Сначала пробуем gzip-версию
    if (LittleFS.exists(gzPath))
    {
        File file = LittleFS.open(gzPath, "r");

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
        return;
    }

    // Fallback на обычный файл
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