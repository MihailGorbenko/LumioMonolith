#include "AnimConfig.hpp"
#include <cstring>

bool AnimConfig::serialize(uint8_t* buf, size_t len) const {
    if (!buf || len < serializedSize()) return false;
    buf[0] = hue;
    return true;
}

bool AnimConfig::deserialize(const uint8_t* buf, size_t len) {
    if (!buf || len < serializedSize()) {
        // Keep current value if insufficient data
        return false;
    }
    hue = buf[0];
    return true;
}
