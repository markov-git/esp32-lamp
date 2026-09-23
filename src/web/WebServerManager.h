#pragma once

#include <WebServer.h>
#include <Arduino.h>
#include <LittleFS.h>

class Api;

class WebServerManager
{
public:
    void begin();
    void handleClient();

private:
    WebServer server;

    void handleNotFound();
    void handleFile();

    String getContentType(const String& path);
};