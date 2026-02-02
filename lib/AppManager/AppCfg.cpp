#include "AppCfg.hpp"
#include <cstring>

bool AppCfg::serialize(uint8_t* buf, size_t len) const {
    if (!buf || len < serializedSize()) return false;
    // Write masterBrightness then powerOn
    std::memcpy(buf, &masterBrightness, sizeof(uint16_t));
    std::memcpy(buf + sizeof(uint16_t), &powerOn, sizeof(uint8_t));
    return true;
}

bool AppCfg::deserialize(const uint8_t* buf, size_t len) {
    if (!buf) return false;
    // Strict format: [masterBrightness:uint16_t][powerOn:uint8_t]
    size_t expected = sizeof(uint16_t) + sizeof(uint8_t);
    if (len < expected) return false;
    std::memcpy(&masterBrightness, buf, sizeof(uint16_t));
    std::memcpy(&powerOn, buf + sizeof(uint16_t), sizeof(uint8_t));
    return true;
}
