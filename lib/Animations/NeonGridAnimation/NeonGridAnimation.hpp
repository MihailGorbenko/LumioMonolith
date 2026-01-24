#ifndef NEON_GRID_ANIMATION_HPP
#define NEON_GRID_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef NEONGRID_DEFAULT_HUE
#define NEONGRID_DEFAULT_HUE 160  // cyan
#endif
#ifndef NEONGRID_DEFAULT_SAT
#define NEONGRID_DEFAULT_SAT 255
#endif
#ifndef NEONGRID_DEFAULT_VAL
#define NEONGRID_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class NeonGridAnimation : public AnimationBase {
public:
	explicit NeonGridAnimation(LedMatrix& m);
	void render() override;

private:
	static const int MAX_W = MATRIX_WIDTH;
};

#endif // NEON_GRID_ANIMATION_HPP
