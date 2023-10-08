#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include "SPIFFS.h"
#include "secrets.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "configs.h"
#include "websocket_handler.h"

unsigned int start = 0;
int interval = 10;
RTC_DS3231 rtc;

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
  DEBUG_PRINTLN(WiFi.localIP());
}

void setup()
{
  if (DEBUG)
  {
    Serial.begin(115200);
  }

  // ------------------------- RTC ------------------------------------
  if (!rtc.begin())
  {
    DEBUG_PRINTLN("Could not find RTC! Check circuit.");
    while (1)
      ;
  }
  DEBUG_PRINTLN("RTC CLOCK initiated correctly");

  // ----------------------- SD card ---------------------------------
  if (!SPIFFS.begin(false))
  {
    DEBUG_PRINTLN("SPIFFS Mount Failed");
    return;
  }
  loadConfigFile();

  // -------------------- Webserver -------------------------------
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
  //   DEBUG_PRINTLN(String(now.toString(buffer)) + ", " + String(rtc.getTemperature()) + "ºC");
  // }
}