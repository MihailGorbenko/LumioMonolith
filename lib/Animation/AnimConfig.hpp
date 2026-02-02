#ifndef ANIM_CONFIG_HPP
#define ANIM_CONFIG_HPP

#include <cstdint>
#include "../StorageManager/Serializable.hpp"

// Animation config: hue only
class AnimConfig : public ISerializable {
public:
    uint8_t hue;

    explicit AnimConfig(uint8_t h = 0) : hue(h) {}

    inline void setHue(uint8_t h) { hue = h; }

    // ISerializable implementation
    size_t serializedSize() const override { return 1; }
    bool serialize(uint8_t* buf, size_t len) const override;
    bool deserialize(const uint8_t* buf, size_t len) override;
};

#endif // ANIM_CONFIG_HPP
