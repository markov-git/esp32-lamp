#include "SdCard.h"

bool SdCard::begin()
{
    SPI.begin(
        SCK_PIN,
        MISO_PIN,
        MOSI_PIN,
        CS_PIN
    );

    if (!SD.begin(CS_PIN, SPI))
    {
        Serial.println("SD initialization failed!");
        ready = false;
        return false;
    }

    ready = true;

    Serial.print("SD initialized. ");

    Serial.print("Card size: ");
    Serial.print(SD.cardSize() / (1024 * 1024));
    Serial.println(" MB");


    return true;
}

bool SdCard::isReady() const
{
    return ready;
}

bool SdCard::exists(const char* path) const
{
    if (!ready)
        return false;

    return SD.exists(path);
}

File SdCard::open(
    const char* path,
    const char* mode
)
{
    if (!ready)
        return File();

    return SD.open(path, mode);
}

bool SdCard::forEachFile(
    const char* directory,
    bool (*callback)(const char* path, void* context),
    void* context
)
{
    if (!ready)
        return false;

    if (callback == nullptr)
        return false;

    File root = SD.open(directory);

    if (!root || !root.isDirectory())
        return false;

    File file = root.openNextFile();

    while (file)
    {
        if (!file.isDirectory())
        {
            const char* path = file.path();

            if (!callback(path, context))
            {
                file.close();
                root.close();
                return true;
            }
        }

        file.close();

        file = root.openNextFile();
    }

    root.close();

    return true;
}

bool SdCard::mkdir(const char* path)
{
    if (!ready)
        return false;

    if (SD.exists(path))
        return true;

    return SD.mkdir(path);
}

bool SdCard::writeFile(
    const char* path,
    const String& content
)
{
    if (!ready)
        return false;

    File file = SD.open(path, FILE_WRITE);

    if (!file)
    {
        Serial.print("Failed to open file for writing: ");
        Serial.println(path);
        return false;
    }

    const size_t written = file.print(content);

    file.close();

    if (written != content.length())
    {
        Serial.print("Failed to write complete file: ");
        Serial.println(path);
        return false;
    }

    return true;
}

bool SdCard::appendFile(
    const char* path,
    const String& content
)
{
    if (!ready)
        return false;

    File file = SD.open(path, FILE_APPEND);

    if (!file)
    {
        Serial.print("Failed to open file for appending: ");
        Serial.println(path);
        return false;
    }

    const size_t written = file.print(content);

    file.close();

    if (written != content.length())
    {
        Serial.print("Failed to append complete data: ");
        Serial.println(path);
        return false;
    }

    return true;
}

bool SdCard::readFile(
    const char* path,
    String& content
)
{
    content = "";

    if (!ready)
        return false;

    File file = SD.open(path, FILE_READ);

    if (!file)
    {
        Serial.print("Failed to open file for reading: ");
        Serial.println(path);
        return false;
    }

    content.reserve(file.size());

    while (file.available())
    {
        content += static_cast<char>(file.read());
    }

    file.close();

    return true;
}

bool SdCard::removeFile(const char* path)
{
    if (!ready)
        return false;

    if (!SD.exists(path))
        return false;

    return SD.remove(path);
}

bool SdCard::listFiles(
    const char* directory,
    std::vector<String>& paths
) const
{
    if (!ready)
        return false;

    File root = SD.open(directory);

    if (!root || !root.isDirectory())
        return false;

    File file = root.openNextFile();

    while (file)
    {
        if (!file.isDirectory())
        {
            paths.emplace_back(file.path());
        }

        file.close();

        file = root.openNextFile();
    }

    root.close();

    return true;
}