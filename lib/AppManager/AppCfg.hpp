#pragma once
#include <cstdint>
#include "../StorageManager/Serializable.hpp"

// App configuration container implementing ISerializable
class AppCfg : public ISerializable {
public:
    uint16_t masterBrightness; // 0..APP_STEPS-1
    uint16_t lastAnimId;       // stable animation ID

    AppCfg() : masterBrightness(0), lastAnimId(0) {}

    // ISerializable
    size_t serializedSize() const override { return 2 * sizeof(uint16_t); }
    bool serialize(uint8_t* buf, size_t len) const override;
    bool deserialize(const uint8_t* buf, size_t len) override;
};
