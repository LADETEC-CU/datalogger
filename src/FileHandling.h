#include <Arduino.h>
#include <FS.h>
#include "SPIFFS.h"
#include "configs.h"

void listDir(fs::FS &fs, const char *dirname, uint8_t levels, DynamicJsonDocument *doc)
{
    DEBUG_PRINT("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        DEBUG_PRINTLN("− failed to open directory");

        return;
    }
    if (!root.isDirectory())
    {
        DEBUG_PRINTLN(" − not a directory");

        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            DEBUG_PRINT("  DIR : ");
            DEBUG_PRINTLN(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1, doc);
            }
        }
        else
        {
            DEBUG_PRINT("  FILE: ");
            DEBUG_PRINT(file.name());
            DEBUG_PRINT("\tSIZE: ");
            DEBUG_PRINTLN(file.size());
            // Populate Json
            char extractedDate[9];
            memcpy(extractedDate, file.name() + 6, 8);
            extractedDate[8] = '\0';
            (*doc)["files"][extractedDate] = file.size();
        }
        file = root.openNextFile();
    }
}

void readFile(fs::FS &fs, const char *path)
{
    DEBUG_PRINT("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory())
    {
        DEBUG_PRINTLN("− failed to open file for reading");

        return;
    }

    DEBUG_PRINTLN("− read from file:");

    while (file.available())
    {
        Serial.write(file.read());
    }
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
    DEBUG_PRINT("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        DEBUG_PRINTLN("− failed to open file for appending");

        return;
    }
    if (file.print(message))
    {
        DEBUG_PRINTLN("− message appended");
    }
    else
    {
        DEBUG_PRINTLN("− append failed");
    }
}

bool deleteFile(fs::FS &fs, const char *path)
{
    DEBUG_PRINT("Deleting file: %s\r\n", path);

    if (fs.remove(path))
    {
        DEBUG_PRINTLN("− file deleted");
        return true;
    }
    else
    {
        DEBUG_PRINTLN("− delete failed");
        return false;
    }
}