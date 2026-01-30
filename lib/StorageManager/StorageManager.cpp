#include "StorageManager.hpp"
#include <Arduino.h>
#include <cstdio>
#include <vector>
#include "../../src/debug.hpp"

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
        LOGF("Storage", "payload too large len=%u (scratch_max=%u) for ns=%s key=%s\n", (unsigned)payloadLen, (unsigned)SCRATCH_MAX, ns, key);
        return false;
    }
    uint8_t* bufPtr = scratch;
    bool okSer = obj.serialize(bufPtr, payloadLen);
    if (!okSer) return false;
    if (!prefs.begin(ns, false)) {
        LOGF("Storage", "begin failed for ns=%s (write)\n", ns);
        return false;
    }
    size_t written = prefs.putBytes(key, bufPtr, payloadLen);
    prefs.end();
    LOGF("Storage", "saveSerializable ns=%s key=%s len=%u written=%u\n", ns, key, (unsigned)payloadLen, (unsigned)written);
    return written == payloadLen;
}

bool StorageManager::loadSerializable(const char* ns, const char* key, ISerializable& obj) {
    if (!ns || !key) return false;
    size_t expected = obj.serializedSize();
    if (expected == 0) return false;
    if (!prefs.begin(ns, true)) {
        LOGF("Storage", "begin failed for ns=%s (read)\n", ns);
        return false;
    }
    size_t storedLen = prefs.getBytesLength(key);
    if (storedLen == 0) { prefs.end(); return false; }
    if (storedLen != expected) {
        LOGF("Storage", "size mismatch ns=%s key=%s expected=%u stored=%u\n", ns, key, (unsigned)expected, (unsigned)storedLen);
    }
    size_t readLen = (storedLen < expected) ? storedLen : expected;
    if (readLen > SCRATCH_MAX) {
        prefs.end();
        LOGF("Storage", "read length too large len=%u (scratch_max=%u) for ns=%s key=%s\n", (unsigned)readLen, (unsigned)SCRATCH_MAX, ns, key);
        return false;
    }
    uint8_t* bufPtr = scratch;
    size_t got = prefs.getBytes(key, bufPtr, readLen);
    prefs.end();
    bool ok = false;
    if (got > 0) {
        ok = obj.deserialize(bufPtr, got);
    }
    LOGF("Storage", "loadSerializable ns=%s key=%s stored=%u read=%u got=%u ok=%d\n", ns, key, (unsigned)storedLen, (unsigned)readLen, (unsigned)got, (int)ok);
    return ok;
}

bool StorageManager::saveAnimation(AnimationBase& anim) {
    // Используем ключ на основе ID, чтобы соблюдать ограничение длины ключа NVS.
    char keyBuf[16]; // Максимальная длина ключа NVS — 15 символов (+ NUL).
    anim.makeNvsKeyById(keyBuf, sizeof(keyBuf));
    if (!keyBuf[0]) return false;
    // Сохраняем только конфигурацию анимации (AnimConfig реализует ISerializable).
    return saveSerializable("anim", keyBuf, anim.getConfig());
}

bool StorageManager::loadAnimation(AnimationBase& anim) {
    // Используем ключ на основе ID, чтобы соблюдать ограничение длины ключа NVS.
    char keyBuf[16]; // Максимальная длина ключа NVS — 15 символов (+ NUL).
    anim.makeNvsKeyById(keyBuf, sizeof(keyBuf));
    if (!keyBuf[0]) return false;
    // Загружаем данные в конфигурацию анимации.
    return loadSerializable("anim", keyBuf, anim.getConfig());
}
