#include <Arduino.h>
#include "configs.h"
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "FileHandling.h"

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        // client connected
        DEBUG_PRINT("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        // client disconnected
        DEBUG_PRINT("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
    }
    else if (type == WS_EVT_ERROR)
    {
        // error was received from the other end
        DEBUG_PRINT("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    }
    else if (type == WS_EVT_PONG)
    {
        // pong message was received (in response to a ping request maybe)
        DEBUG_PRINT("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    }
    else if (type == WS_EVT_DATA)
    {
        // data packet
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len)
        {
            // the whole message is in a single frame and we got all of it's data
            if (info->opcode == WS_TEXT)
            {
                data[info->len] = '\0';

                DEBUG_PRINT("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
                DEBUG_PRINTLN((char *)data);

                // Create JSON request object
                DynamicJsonDocument doc(1024);
                // Create JSON response object
                DynamicJsonDocument responseDoc(1024);

                // Parse request
                DeserializationError error = deserializeJson(doc, (char *)data);
                if (error)
                {
                    if (DEBUG)
                    {
                        DEBUG_PRINT("deserializeJson() failed: ");
                        DEBUG_PRINTLN(error.c_str());
                    }
                    return;
                }

                // We must receive always a command
                if (doc.containsKey("cmd"))
                {
                    const char *cmd = doc["cmd"];
                    DEBUG_PRINT("Command: %s\n", cmd);

                    // ------------------------------- Get status ----------------------------
                    if (strcmp("get_status", cmd) == 0)
                    {
                        // Datetime
                        char buffer[21];
                        DateTime now = rtc.now();
                        strcpy(buffer, "DD MMM YYYY hh:mm:ss");
                        now.toString(buffer);
                        responseDoc["ack"]["datetime"] = buffer;
                        // Interval
                        responseDoc["ack"]["interval"] = interval;
                        // TODO Modbus status
                        // TODO SD card status
                        // TODO other status
                    }

                    // ------------------------------- Set time ----------------------------
                    else if (strcmp("set_time", cmd) == 0)
                    {
                        if (doc.containsKey("date") && doc.containsKey("time"))
                        {
                            const char *date = doc["date"];
                            const char *time = doc["time"];
                            rtc.adjust(DateTime(date, time));
                            responseDoc["ack"] = "Time updated successfully!";
                        }
                        else
                        {
                            responseDoc["error"] = "Please provide 'date' and 'time' keys for the set_time command";
                        }
                    }

                    // --------------------------------- Get files --------------------------
                    else if (strcmp("get_files", cmd) == 0)
                    {
                        DynamicJsonDocument filesDoc(1024);
                        listDir(SPIFFS, "/logs", 1, &filesDoc);
                        responseDoc["ack"]["files"] = filesDoc;
                    }

                    // -------------------------------- Delete file --------------------------
                    else if (strcmp("rm_file", cmd) == 0)
                    {
                        if (doc.containsKey("filename"))
                        {
                            const char *filename = doc["filename"];
                            // Delete the file
                            if (!deleteFile(SPIFFS, filename))
                            {
                                responseDoc["error"] = "Error deleting file!";
                                responseDoc["filename"] = filename;
                            }
                            else
                            {
                                DynamicJsonDocument filesDoc(1024);
                                listDir(SPIFFS, "/logs", 1, &filesDoc);
                                responseDoc["ack"]["files"] = filesDoc;
                            }
                        }
                        else
                        {
                            responseDoc["error"] = "Please provide 'filename' key for the rm_file command";
                        }
                    }

                    // ------------------------------- Set interval ----------------------------
                    else if (strcmp("set_interval", cmd) == 0)
                    {
                        if (doc.containsKey("interval"))
                        {
                            interval = doc["interval"];
                            if (updateConfigFile(interval))
                            {
                                responseDoc["ack"] = "Interval updated successfully!";
                            }
                            else
                            {
                                responseDoc["error"] = "Error updating interval";
                            }
                        }
                        else
                        {
                            responseDoc["error"] = "Please provide 'interval' key for the set_interval command";
                        }
                    }
                }
                else
                {
                    responseDoc["error"] = "Please provide a command using the 'cmd' key";
                }

                // Convert response to JSON string
                String responseString;
                serializeJson(responseDoc, responseString);
                DEBUG_PRINTLN(responseString);

                // Send JSON response to client
                client->text(responseString);
            }
        }
    }
}
