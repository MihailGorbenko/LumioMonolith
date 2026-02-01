#pragma once
#include <Arduino.h>

// Single global debug flag for the entire application
#ifndef APP_DEBUG
#define APP_DEBUG 1
#endif

// Unified logging enable
#ifndef LOG_ENABLED
#define LOG_ENABLED (APP_DEBUG)
#endif

#if LOG_ENABLED
  #define LOG_PRINT(s) Serial.print(s)
  #define LOG_PRINTLN(s) Serial.println(s)
  #define LOGF(TAG, FMT, ...) Serial.printf("[%s] " FMT, TAG, ##__VA_ARGS__)
#else
  #define LOG_PRINT(s) do {} while(0)
  #define LOG_PRINTLN(s) do {} while(0)
  #define LOGF(TAG, FMT, ...) do {} while(0)
#endif
