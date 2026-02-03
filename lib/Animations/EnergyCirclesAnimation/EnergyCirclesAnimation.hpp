#ifndef ENERGYCIRCLES_ANIMATION_HPP
#define ENERGYCIRCLES_ANIMATION_HPP

#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef ENERGY_DEFAULT_HUE
#define ENERGY_DEFAULT_HUE 160
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

class EnergyCirclesAnimation : public AnimationBase {
public:
    explicit EnergyCirclesAnimation(uint16_t id, LedMatrix& m);
    void render() override;

    const char* getName() const override { return ENERGYCIRCLES_ANIMATION_NAME; }
};

#endif // ENERGYCIRCLES_ANIMATION_HPP
