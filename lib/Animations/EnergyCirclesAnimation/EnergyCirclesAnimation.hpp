#ifndef ENERGYCIRCLES_ANIMATION_HPP
#define ENERGYCIRCLES_ANIMATION_HPP

#include "../../Animation/Animation.hpp"

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

class EnergyCirclesAnimation : public AnimationBase {
public:
    explicit EnergyCirclesAnimation(LedMatrix& m);
    void render() override;
};

#endif // ENERGYCIRCLES_ANIMATION_HPP