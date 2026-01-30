#include "AppCfg.hpp"
#include <cstring>

bool AppCfg::serialize(uint8_t* buf, size_t len) const {
    if (!buf || len < serializedSize()) return false;
    // Write masterBrightness, lastAnimId, then powerOn
    std::memcpy(buf, &masterBrightness, sizeof(uint16_t));
    std::memcpy(buf + sizeof(uint16_t), &lastAnimId, sizeof(uint16_t));
    std::memcpy(buf + 2 * sizeof(uint16_t), &powerOn, sizeof(uint8_t));
    return true;
}

bool AppCfg::deserialize(const uint8_t* buf, size_t len) {
    if (!buf) return false;
    // Support backward compatibility: old payload had only two uint16_t values
    size_t baseSize = 2 * sizeof(uint16_t);
    if (len < baseSize) return false;
    std::memcpy(&masterBrightness, buf, sizeof(uint16_t));
    std::memcpy(&lastAnimId, buf + sizeof(uint16_t), sizeof(uint16_t));
    if (len >= baseSize + sizeof(uint8_t)) {
        std::memcpy(&powerOn, buf + baseSize, sizeof(uint8_t));
    } else {
        // Default to ON if legacy payload did not store power state
        powerOn = 1;
    }
    return true;
}
