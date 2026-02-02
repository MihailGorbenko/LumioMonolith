#include "AnimMngrCfg.hpp"
#include <cstring>

bool AnimMngrCfg::serialize(uint8_t* buf, size_t len) const {
    if (!buf || len < serializedSize()) return false;
    std::memcpy(buf, &lastAnimId, sizeof(uint16_t));
    return true;
}

bool AnimMngrCfg::deserialize(const uint8_t* buf, size_t len) {
    if (!buf || len < serializedSize()) return false;
    std::memcpy(&lastAnimId, buf, sizeof(uint16_t));
    return true;
}
