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
	explicit SparkleWaveAnimation(uint16_t id, LedMatrix& m);
	void render() override;

	// activation hook: cache matrix size and prepare state
	void onActivate() override;
	const char* getName() const override { return SPARKLEWAVE_ANIMATION_NAME; }

private:
	uint8_t sparkleChance; // 0..255

	// cached matrix size populated in onActivate()
	int cachedWidth = 0;
	int cachedHeight = 0;

};

#endif // SPARKLE_WAVE_ANIMATION_HPP
