#ifndef RAINBOW_CHASE_ANIMATION_HPP
#define RAINBOW_CHASE_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

// Default configuration (can be overridden at compile time)
#ifndef RAINBOWCHASE_DEFAULT_HUE
#define RAINBOWCHASE_DEFAULT_HUE 0
#endif
#ifndef RAINBOWCHASE_DEFAULT_SAT
#define RAINBOWCHASE_DEFAULT_SAT 255
#endif
#ifndef RAINBOWCHASE_DEFAULT_VAL
#define RAINBOWCHASE_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class RainbowChaseAnimation : public AnimationBase {
public:
	explicit RainbowChaseAnimation(LedMatrix& m);
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
	void render() override;
};

#endif // RAINBOW_CHASE_ANIMATION_HPP
