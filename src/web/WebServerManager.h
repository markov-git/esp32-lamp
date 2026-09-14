#pragma once

#include <WebServer.h>

class Api;

class WebServerManager
{
public:
    explicit WebServerManager(Api& api);

    void begin();
    void handleClient();

private:
    WebServer server;
    Api& api;

    void handleNotFound();
    void handleFile();

    String getContentType(const String& path);
};