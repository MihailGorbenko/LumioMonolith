#include "StorageManager.hpp"
#include <Arduino.h>
#include "../../src/debug.hpp"

// Include full Animation definition only in cpp to avoid header cyclic dependency
#include "../Animation/Animation.hpp"
// (No need to include AnimConfig; StorageManager operates on ISerializable)

bool StorageManager::saveApp(const ISerializable& obj) {
    return saveSerializable("appV1", "cfg", obj);
}

bool StorageManager::loadApp(ISerializable& obj) {
    return loadSerializable("appV1", "cfg", obj);
}


bool StorageManager::saveSerializable(const char* ns, const char* key, const ISerializable& obj) {
    if (!ns || !key) return false;
    size_t payloadLen = obj.serializedSize();
    if (payloadLen == 0) return false;
    if (payloadLen > StorageManager::SCRATCH_MAX) {
        LOGF("Storage", "payload too large len=%u (scratch_max=%u) for ns=%s key=%s\n", (unsigned)payloadLen, (unsigned)StorageManager::SCRATCH_MAX, ns, key);
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
        prefs.end();
        LOGF("Storage", "size mismatch (abort) ns=%s key=%s expected=%u stored=%u\n", ns, key, (unsigned)expected, (unsigned)storedLen);
        return false;
    }
    size_t readLen = expected;
    if (readLen > StorageManager::SCRATCH_MAX) {
        prefs.end();
        LOGF("Storage", "read length too large len=%u (scratch_max=%u) for ns=%s key=%s\n", (unsigned)readLen, (unsigned)StorageManager::SCRATCH_MAX, ns, key);
        return false;
    }
    uint8_t* bufPtr = scratch;
    size_t got = prefs.getBytes(key, bufPtr, readLen);
    prefs.end();
    bool ok = false;
    if (got == readLen) {
        ok = obj.deserialize(bufPtr, got);
    } else {
        LOGF("Storage", "loadSerializable: partial/failed read ns=%s key=%s expected=%u got=%u\n", ns, key, (unsigned)readLen, (unsigned)got);
    }
    LOGF("Storage", "loadSerializable ns=%s key=%s stored=%u read=%u got=%u ok=%d\n", ns, key, (unsigned)storedLen, (unsigned)readLen, (unsigned)got, (int)ok);
    return ok;
}

bool StorageManager::saveAnimMngrCfg(const ISerializable& obj) {
    return saveSerializable("anim_mngr", "cfg", obj);
}

bool StorageManager::loadAnimMngrCfg(ISerializable& obj) {
    return loadSerializable("anim_mngr", "cfg", obj);
}

bool StorageManager::saveAnimation(AnimationBase& anim) {
    // Use an ID-based key to satisfy NVS key length limits.
    char keyBuf[16]; // Maximum NVS key length is 15 characters (+ NUL).
    anim.makeNvsKeyById(keyBuf, sizeof(keyBuf));
    if (!keyBuf[0]) return false;
    // Save only the animation config (AnimConfig implements ISerializable).
    return saveSerializable("anim", keyBuf, anim.getConfig());
}

bool StorageManager::loadAnimation(AnimationBase& anim) {
    // Use an ID-based key to satisfy NVS key length limits.
    char keyBuf[16]; // Maximum NVS key length is 15 characters (+ NUL).
    anim.makeNvsKeyById(keyBuf, sizeof(keyBuf));
    if (!keyBuf[0]) return false;
    // Load data into the animation configuration.
    return loadSerializable("anim", keyBuf, anim.getConfig());
}
