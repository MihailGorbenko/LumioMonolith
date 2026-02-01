#ifndef RAINBOW_CHASE_ANIMATION_HPP
#define RAINBOW_CHASE_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

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

// Readable name
#ifndef RAINBOWCHASE_ANIMATION_NAME
#define RAINBOWCHASE_ANIMATION_NAME "Rainbow Chase"
#endif

class RainbowChaseAnimation : public AnimationBase {
public:
	explicit RainbowChaseAnimation(uint16_t id);
	void render(LedMatrix& m) override;
	const char* getName() const override { return RAINBOWCHASE_ANIMATION_NAME; }
};

#endif // RAINBOW_CHASE_ANIMATION_HPP
