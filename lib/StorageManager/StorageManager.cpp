#include "StorageManager.hpp"
#include <Arduino.h>
#include <vector>
#include <cstdio>
#include "../../src/AppConfig.hpp"

void StorageManager::begin(const char* ns, bool readOnly) {
    prefs.begin(ns, readOnly);
    #if MODE_DEBUG
    Serial.printf("[Storage] begin ns=%s ro=%d\n", ns ? ns : "(null)", (int)readOnly);
    #endif
}

void StorageManager::end() {
    prefs.end();
    #if MODE_DEBUG
    Serial.println("[Storage] end");
    #endif
}

bool StorageManager::saveApp(int currentIndex, int brightStep) {
    // Requires begin() to be called beforehand
    prefs.putUInt("lastAnim", (uint32_t)currentIndex);
    prefs.putUShort("brightStep", (uint16_t)brightStep);
    #if MODE_DEBUG
    Serial.printf("[Storage] saveApp anim=%d bright=%d\n", currentIndex, brightStep);
    #endif
    return true;
}

bool StorageManager::loadApp(int &outIndex, int &outBrightStep, int defaultIndex, int defaultBright) {
    // Requires begin() to be called beforehand
    uint32_t ai = prefs.getUInt("lastAnim", (uint32_t)defaultIndex);
    uint16_t bs = prefs.getUShort("brightStep", (uint16_t)defaultBright);
    outIndex = (int)ai;
    outBrightStep = (int)bs;
    #if MODE_DEBUG
    Serial.printf("[Storage] loadApp anim=%d bright=%d (defaults: %d,%d)\n", outIndex, outBrightStep, defaultIndex, defaultBright);
    #endif
    return true;
}

bool StorageManager::saveSerializable(const char* key, const ISerializable& obj) {
    if (!key) return false;
    size_t payloadLen = obj.serializedSize();
    if (payloadLen == 0) return false;
    std::vector<uint8_t> buf;
    buf.resize(payloadLen);
    if (!obj.serialize(buf.data(), payloadLen)) return false;
    size_t written = prefs.putBytes(key, buf.data(), buf.size());
    #if MODE_DEBUG
    Serial.printf("[Storage] saveSerializable key=%s len=%u written=%u\n", key, (unsigned)buf.size(), (unsigned)written);
    #endif
    return written == buf.size();
}

bool StorageManager::loadSerializable(const char* key, ISerializable& obj) {
    if (!key) return false;
    size_t expected = obj.serializedSize();
    if (expected == 0) return false;
    size_t storedLen = prefs.getBytesLength(key);
    if (storedLen == 0) return false;
    size_t readLen = (storedLen < expected) ? storedLen : expected;
    std::vector<uint8_t> buf;
    buf.resize(readLen);
    size_t got = prefs.getBytes(key, buf.data(), buf.size());
    if (got == 0) return false;
    #if MODE_DEBUG
    Serial.printf("[Storage] loadSerializable key=%s stored=%u read=%u got=%u\n", key, (unsigned)storedLen, (unsigned)readLen, (unsigned)got);
    #endif
    return obj.deserialize(buf.data(), got);
}

bool StorageManager::saveAnimConfig(int index, const ISerializable& obj) {
    char key[16];
    std::snprintf(key, sizeof(key), "a%d", index);
    #if MODE_DEBUG
    Serial.printf("[Storage] saveAnimConfig idx=%d key=%s\n", index, key);
    #endif
    return saveSerializable(key, obj);
}

bool StorageManager::loadAnimConfig(int index, ISerializable& obj) {
    char key[16];
    std::snprintf(key, sizeof(key), "a%d", index);
    #if MODE_DEBUG
    Serial.printf("[Storage] loadAnimConfig idx=%d key=%s\n", index, key);
    #endif
    return loadSerializable(key, obj);
}
