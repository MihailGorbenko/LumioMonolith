#include "AppCfg.hpp"
#include <cstring>

bool AppCfg::serialize(uint8_t* buf, size_t len) const {
    if (!buf || len < serializedSize()) return false;
    // Write two uint16_t values sequentially
    std::memcpy(buf, &masterBrightness, sizeof(uint16_t));
    std::memcpy(buf + sizeof(uint16_t), &lastAnimId, sizeof(uint16_t));
    return true;
}

bool AppCfg::deserialize(const uint8_t* buf, size_t len) {
    if (!buf || len < serializedSize()) return false;
    std::memcpy(&masterBrightness, buf, sizeof(uint16_t));
    std::memcpy(&lastAnimId, buf + sizeof(uint16_t), sizeof(uint16_t));
    return true;
}
