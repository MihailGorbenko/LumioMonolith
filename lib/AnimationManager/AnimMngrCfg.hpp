#pragma once
#include <cstdint>
#include "../StorageManager/Serializable.hpp"

// Animation manager configuration container implementing ISerializable
class AnimMngrCfg : public ISerializable {
public:
    uint16_t lastAnimId; // stable animation ID

    AnimMngrCfg() : lastAnimId(1) {}

    // ISerializable
    size_t serializedSize() const override { return sizeof(uint16_t); }
    bool serialize(uint8_t* buf, size_t len) const override;
    bool deserialize(const uint8_t* buf, size_t len) override;
};
