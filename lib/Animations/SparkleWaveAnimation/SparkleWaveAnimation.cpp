#include "SparkleWaveAnimation.hpp"
#include <FastLED.h>

SparkleWaveAnimation::SparkleWaveAnimation(uint16_t id)
		: AnimationBase(SPARKLEWAVE_DEFAULT_HUE, id),
			sparkleChance(28) {}

void SparkleWaveAnimation::render(LedMatrix& m) {
	m.clear();

	int w = m.getWidth();
	int hgt = m.getHeight();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	uint32_t now = millis();
	uint32_t t32 = now / 5U;
	uint8_t t = (uint8_t)(t32 & 0xFF);

	for (int x = 0; x < w; ++x) {
		uint8_t wave = sin8((uint8_t)(t + x * 10));
		// allow full brightness at wave peaks
		uint8_t vWave = scale8(ANIMATION_DEFAULT_VAL, qadd8(0, wave));
		uint8_t hOut = (uint8_t)(animCfg.hue + (wave >> 2));

		// Column-level sparkle decision (removes per-pixel random())
		uint8_t sparkleBoost = ((((uint8_t)((now >> 3) + x * 73)) & 0xFF) < sparkleChance) ? 120 : 0;

		for (int y = 0; y < hgt; ++y) {
			// Add slight vertical variation
			uint8_t vOut = (hgt >= 2) ? ((y == 0) ? vWave : scale8(vWave, 200)) : vWave;
			// Occasional sparkle boost without per-pixel RNG
			if (sparkleBoost) vOut = qadd8(vOut, sparkleBoost);
			m.setPixelHSV(x, y, hOut, ANIMATION_DEFAULT_SAT, vOut);
		}
	}

	// show() is managed by AppController
}

// Base class provides ISerializable
