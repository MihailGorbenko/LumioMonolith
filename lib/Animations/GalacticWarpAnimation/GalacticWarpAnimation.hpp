#ifndef GALACTIC_WARP_ANIMATION_HPP
#define GALACTIC_WARP_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef GALACTICWARP_DEFAULT_HUE
#define GALACTICWARP_DEFAULT_HUE 240  // blue/purple
#endif
#ifndef GALACTICWARP_DEFAULT_SAT
#define GALACTICWARP_DEFAULT_SAT 220
#endif
#ifndef GALACTICWARP_DEFAULT_VAL
#define GALACTICWARP_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class GalacticWarpAnimation : public AnimationBase {
public:
	explicit GalacticWarpAnimation(LedMatrix& m);
	void render() override;
private:
    static const int MAX_W = MATRIX_WIDTH;
};

#endif // GALACTIC_WARP_ANIMATION_HPP
