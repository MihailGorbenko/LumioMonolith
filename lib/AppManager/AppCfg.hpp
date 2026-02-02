#pragma once
#include <cstdint>
#include "../StorageManager/Serializable.hpp"

// App configuration container implementing ISerializable
class AppCfg : public ISerializable {
public:
    uint16_t masterBrightness; // 0..255 (linear input, gamma applied at runtime)
    uint8_t powerOn;           // 0=off, 1=on (restore power state)

    AppCfg() : masterBrightness(0), powerOn(0) {}

    // ISerializable
    size_t serializedSize() const override { return sizeof(uint16_t) + sizeof(uint8_t); }
    bool serialize(uint8_t* buf, size_t len) const override;
    bool deserialize(const uint8_t* buf, size_t len) override;
};
