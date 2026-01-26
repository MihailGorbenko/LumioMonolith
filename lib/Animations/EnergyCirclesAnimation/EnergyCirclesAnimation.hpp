#ifndef ENERGYCIRCLES_ANIMATION_HPP
#define ENERGYCIRCLES_ANIMATION_HPP

#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef ENERGY_DEFAULT_HUE
#define ENERGY_DEFAULT_HUE 0
#endif

#ifndef ENERGY_DEFAULT_SAT
#define ENERGY_DEFAULT_SAT 255
#endif

#ifndef ENERGY_DEFAULT_VAL
#define ENERGY_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Length of the moving segment per row
#ifndef ENERGY_SEGMENT_LEN
#define ENERGY_SEGMENT_LEN 9
#endif

// Movement speed in milliseconds per step
#ifndef ENERGY_SPEED_MS
#define ENERGY_SPEED_MS 80
#endif

// Readable name
#ifndef ENERGYCIRCLES_ANIMATION_NAME
#define ENERGYCIRCLES_ANIMATION_NAME "Energy Circles"
#endif

class EnergyCirclesAnimation : public AnimationBase, public ISerializable {
public:
    explicit EnergyCirclesAnimation(LedMatrix& m);
    void render() override;

    // ISerializable
    size_t serializedSize() const override { return 2; }
    bool serialize(uint8_t* out, size_t maxLen) const override;
    bool deserialize(const uint8_t* data, size_t len) override;
    ISerializable* serializable() override { return this; }
};

#endif // ENERGYCIRCLES_ANIMATION_HPP