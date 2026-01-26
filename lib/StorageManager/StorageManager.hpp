#pragma once
#include <Preferences.h>
#include "Serializable.hpp"

class StorageManager {
public:
    void begin(const char* ns = "app", bool readOnly = false);
    void end();

    bool saveApp(int currentIndex, int brightStep);
    bool loadApp(int &outIndex, int &outBrightStep, int defaultIndex = 0, int defaultBright = 10);

    bool saveSerializable(const char* key, const ISerializable& obj);
    bool loadSerializable(const char* key, ISerializable& obj);

    bool saveAnimConfig(int index, const ISerializable& obj);
    bool loadAnimConfig(int index, ISerializable& obj);

private:
    Preferences prefs;
};
