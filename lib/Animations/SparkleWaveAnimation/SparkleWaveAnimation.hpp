#ifndef SPARKLE_WAVE_ANIMATION_HPP
#define SPARKLE_WAVE_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef SPARKLEWAVE_DEFAULT_HUE
#define SPARKLEWAVE_DEFAULT_HUE 96
#endif
#ifndef SPARKLEWAVE_DEFAULT_SAT
#define SPARKLEWAVE_DEFAULT_SAT 255
#endif
#ifndef SPARKLEWAVE_DEFAULT_VAL
#define SPARKLEWAVE_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class SparkleWaveAnimation : public AnimationBase {
public:
	explicit SparkleWaveAnimation(LedMatrix& m);
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
	void render() override;

private:
	uint8_t sparkleChance; // 0..255
};

#endif // SPARKLE_WAVE_ANIMATION_HPP
