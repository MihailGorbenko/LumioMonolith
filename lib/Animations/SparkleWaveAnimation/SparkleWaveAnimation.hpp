#ifndef SPARKLE_WAVE_ANIMATION_HPP
#define SPARKLE_WAVE_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef SPARKLEWAVE_DEFAULT_HUE
#define SPARKLEWAVE_DEFAULT_HUE 96
#endif
#ifndef SPARKLEWAVE_DEFAULT_SAT
#define SPARKLEWAVE_DEFAULT_SAT 255
#endif
#ifndef SPARKLEWAVE_DEFAULT_VAL
#define SPARKLEWAVE_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Readable name
#ifndef SPARKLEWAVE_ANIMATION_NAME
#define SPARKLEWAVE_ANIMATION_NAME "Sparkle Wave"
#endif

class SparkleWaveAnimation : public AnimationBase {
public:
	explicit SparkleWaveAnimation(LedMatrix& m);
	void render() override;

private:
	uint8_t sparkleChance; // 0..255

	// ISerializable
public:
	const char* getName() const override { return SPARKLEWAVE_ANIMATION_NAME; }
	const char* getNvsKeyName() const override { return "sparkle"; }
};

#endif // SPARKLE_WAVE_ANIMATION_HPP
