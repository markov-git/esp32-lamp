#include "FileServer.h"
#include <LittleFS.h>

void setupFileServer()
{
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount failed!");
        return;
    }
    else {
        Serial.println("LittleFS mounted");
    }
}