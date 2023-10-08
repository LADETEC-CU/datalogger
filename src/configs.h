#include "RTClib.h"
extern RTC_DS3231 rtc;
extern AsyncWebSocket ws;
extern int interval;

#define DEBUG true
#define DEBUG_PRINT(...)                \
    do                                  \
    {                                   \
        if (DEBUG)                      \
        {                               \
            Serial.printf(__VA_ARGS__); \
        }                               \
    } while (0)
#define DEBUG_PRINTLN(...)               \
    do                                   \
    {                                    \
        if (DEBUG)                       \
        {                                \
            Serial.println(__VA_ARGS__); \
        }                                \
    } while (0)