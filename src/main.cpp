#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"

RTC_DS3231 rtc;
char buffer[256];

void setup()
{
  Serial.begin(115200);

  if (!rtc.begin())
  {
    Serial.println("Could not find RTC! Check circuit.");
    while (1)
      ;
  }

  // rtc.adjust(DateTime(__DATE__, __TIME__));
  Serial.println("RTC CLOCK");
  delay(5000);
}

void loop()
{
  DateTime now = rtc.now();
  strcpy(buffer, "DDD, DD MMM YYYY hh:mm:ss");

  // Appelle la fonction toString et affiche le résultat sur le moniteur série
  Serial.println(now.toString(buffer));

  delay(2000);
}