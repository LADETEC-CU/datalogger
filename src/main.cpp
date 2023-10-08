#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include "SPIFFS.h"
#include "RTClib.h"
#include "secrets.h"
#include "FileHandling.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"

#define DEBUG true

unsigned int start = 0;

RTC_DS3231 rtc;
char buffer[21];

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void testHandler(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument doc(1024);
  doc["heap"] = ESP.getFreeHeap();
  doc["ssid"] = WiFi.SSID();
  serializeJson(doc, *response);
  request->send(response);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    // client connected
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    // client disconnected
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  }
  else if (type == WS_EVT_ERROR)
  {
    // error was received from the other end
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  }
  else if (type == WS_EVT_PONG)
  {
    // pong message was received (in response to a ping request maybe)
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
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

        if (DEBUG)
        {
          Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
          Serial.println((char *)data);
        }
        DynamicJsonDocument doc(1024);

        DeserializationError error = deserializeJson(doc, (char *)data);
        if (error)
        {
          if (DEBUG)
          {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
          }
          return;
        }
        if (doc.containsKey("cmd"))
        {
          int result; // For string comparisons

          const char *cmd = doc["cmd"];
          if (DEBUG)
          {
            Serial.println(cmd);
          }
          // Get time
          result = strcmp("get_time", cmd);

          if (result == 0)
          {
            DateTime now = rtc.now();
            strcpy(buffer, "DD MMM YYYY hh:mm:ss");
            client->text(now.toString(buffer));
          }
          // Set time
          result = strcmp("set_time", cmd);

          if (result == 0)
          {
            if (doc.containsKey("date") && doc.containsKey("time"))
            {
              const char *date = doc["date"];
              const char *time = doc["time"];
              rtc.adjust(DateTime(date, time));
            }
            else
            {
              client->text("Please, for the set_time command, provide 'date' and 'time' keys(i.e.\n'date': 'Apr 16 2020',\n'time': '18:34:56') ");
            }
          }
        }
      else
      {
        client->text("Please provide a command using the 'cmd' key");
      }
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

  if (!rtc.begin())
  {
    Serial.println("Could not find RTC! Check circuit.");
    while (1)
      ;
  }
  if (!SPIFFS.begin(false))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  // rtc.adjust(DateTime(__DATE__, __TIME__));
  Serial.println("RTC CLOCK initiated correctly");

  initWiFi();

  // WebSocket handler
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.serveStatic("/", SPIFFS, "/www/");
  server.serveStatic("/logs/", SPIFFS, "/logs/");

  server.begin();
  start = millis();
}

void loop()
{
  // if (millis() - start > 2000)
  // {
  //   start = millis();
  //   DateTime now = rtc.now();
  //   strcpy(buffer, "DD MMM YYYY hh:mm:ss");
  //   Serial.println(String(now.toString(buffer)) + ", " + String(rtc.getTemperature()) + "ºC");
  // }
}