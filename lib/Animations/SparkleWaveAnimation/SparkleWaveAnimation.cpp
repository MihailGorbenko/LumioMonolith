#include "SparkleWaveAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>

SparkleWaveAnimation::SparkleWaveAnimation(uint16_t id, LedMatrix& m)
		: AnimationBase(SPARKLEWAVE_DEFAULT_HUE, id, m),
			sparkleChance(0), cachedWidth(0), cachedHeight(0) {}

void SparkleWaveAnimation::onActivate() {
	// Cache matrix dimensions to avoid calling getWidth/getHeight each frame.
	LedMatrix& m = matrix;
	int w = m.getWidth();
	int h = m.getHeight();
	if (w <= 0) w = 1;
	if (h <= 0) h = 1;
	cachedWidth = w;
	cachedHeight = h;
	AnimationBase::onActivate();
}

void SparkleWaveAnimation::render() {
	LedMatrix& m = matrix;
	m.clear();

	// Use cached dimensions populated in onActivate(); assume prepared.
	int w = cachedWidth;
	int hgt = cachedHeight;

	uint32_t now = millis();
	uint32_t t32 = now / 5U;
	uint8_t t = (uint8_t)(t32 & 0xFF);

	for (int x = 0; x < w; ++x) {
		uint8_t wave = sin8((uint8_t)(t + x * 10));
		// allow full brightness at wave peaks
		uint8_t vWave = scale8(ANIMATION_DEFAULT_VAL, qadd8(0, wave));
		uint8_t hOut = (uint8_t)(animCfg.hue + (wave >> 2));


		for (int y = 0; y < hgt; ++y) {
			// Add slight vertical variation
			uint8_t vOut = (hgt >= 2) ? ((y == 0) ? vWave : scale8(vWave, 200)) : vWave;
			// No sparkle boost — flashes removed
			m.setPixelHSV(x, y, hOut, ANIMATION_DEFAULT_SAT, vOut);
		}
	}


}

// Base class provides ISerializable
