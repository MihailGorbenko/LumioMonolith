#include "StorageManager.hpp"
#include <Arduino.h>
#include <cstdio>
#include <vector>
#include "../../src/AppConfig.hpp"

bool StorageManager::saveApp(const ISerializable& obj) {
    return saveSerializable("app", "cfg", obj);
}

bool StorageManager::loadApp(ISerializable& obj) {
    return loadSerializable("app", "cfg", obj);
}

bool StorageManager::saveSerializable(const char* ns, const char* key, const ISerializable& obj) {
    if (!ns || !key) return false;
    size_t payloadLen = obj.serializedSize();
    if (payloadLen == 0) return false;
    if (payloadLen > SCRATCH_MAX) {
        #if MODE_DEBUG
        Serial.printf("[Storage] payload too large len=%u (scratch_max=%u) for ns=%s key=%s\n", (unsigned)payloadLen, (unsigned)SCRATCH_MAX, ns, key);
        #endif
        return false;
    }
    uint8_t* bufPtr = scratch;
    bool okSer = obj.serialize(bufPtr, payloadLen);
    if (!okSer) return false;
    if (!prefs.begin(ns, false)) {
        #if MODE_DEBUG
        Serial.printf("[Storage] begin failed for ns=%s (write)\n", ns);
        #endif
        return false;
    }
    size_t written = prefs.putBytes(key, bufPtr, payloadLen);
    prefs.end();
    #if MODE_DEBUG
    Serial.printf("[Storage] saveSerializable ns=%s key=%s len=%u written=%u\n", ns, key, (unsigned)payloadLen, (unsigned)written);
    #endif
    return written == payloadLen;
}

bool StorageManager::loadSerializable(const char* ns, const char* key, ISerializable& obj) {
    if (!ns || !key) return false;
    size_t expected = obj.serializedSize();
    if (expected == 0) return false;
    if (!prefs.begin(ns, true)) {
        #if MODE_DEBUG
        Serial.printf("[Storage] begin failed for ns=%s (read)\n", ns);
        #endif
        return false;
    }
    size_t storedLen = prefs.getBytesLength(key);
    if (storedLen == 0) { prefs.end(); return false; }
    if (storedLen != expected) {
        #if MODE_DEBUG
        Serial.printf("[Storage] size mismatch ns=%s key=%s expected=%u stored=%u\n", ns, key, (unsigned)expected, (unsigned)storedLen);
        #endif
    }
    size_t readLen = (storedLen < expected) ? storedLen : expected;
    if (readLen > SCRATCH_MAX) {
        prefs.end();
        #if MODE_DEBUG
        Serial.printf("[Storage] read length too large len=%u (scratch_max=%u) for ns=%s key=%s\n", (unsigned)readLen, (unsigned)SCRATCH_MAX, ns, key);
        #endif
        return false;
    }
    uint8_t* bufPtr = scratch;
    size_t got = prefs.getBytes(key, bufPtr, readLen);
    prefs.end();
    bool ok = false;
    if (got > 0) {
        ok = obj.deserialize(bufPtr, got);
    }
    #if MODE_DEBUG
    Serial.printf("[Storage] loadSerializable ns=%s key=%s stored=%u read=%u got=%u ok=%d\n", ns, key, (unsigned)storedLen, (unsigned)readLen, (unsigned)got, (int)ok);
    #endif
    return ok;
}

bool StorageManager::saveAnimation(AnimationBase& anim) {
    // Use ID-based key to respect NVS key length constraints
    char keyBuf[16]; // NVS key max length is 15 chars (+ NUL)
    anim.makeNvsKeyById(keyBuf, sizeof(keyBuf));
    if (!keyBuf[0]) return false;
    // Persist only the animation's config (AnimConfig implements ISerializable)
    return saveSerializable("anim", keyBuf, anim.getConfig());
}

bool StorageManager::loadAnimation(AnimationBase& anim) {
    // Use ID-based key to respect NVS key length constraints
    char keyBuf[16]; // NVS key max length is 15 chars (+ NUL)
    anim.makeNvsKeyById(keyBuf, sizeof(keyBuf));
    if (!keyBuf[0]) return false;
    // Load into the animation's config
    return loadSerializable("anim", keyBuf, anim.getConfig());
}
