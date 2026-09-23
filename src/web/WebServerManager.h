#pragma once

#include <WebServer.h>

class WebServerManager
{
public:
    explicit WebServerManager(WebServer& server);

    void begin();

private:
    WebServer& server;

    void registerRoutes();

    void handleRoot();
    void handleStaticFile();
    void handleNotFound();

    String getContentType(const String& path);
};