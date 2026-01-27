#include "PulseWaveAnimation.hpp"
#include <FastLED.h>

PulseWaveAnimation::PulseWaveAnimation(LedMatrix& m)
	: AnimationBase(m, PULSEWAVE_DEFAULT_HUE, PULSEWAVE_DEFAULT_SAT),
	  pulseRadius(0) {}

void PulseWaveAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	// Cosmic pulse expanding from center
	uint32_t now = millis();
	uint32_t t32 = now / 6U;
	uint8_t t = (uint8_t)(t32 & 0xFF);
	uint8_t tPhase = (uint8_t)((t32 + 128U) & 0xFF);
	int centerX = w / 2;

	for (int x = 0; x < w; ++x) {
		// Distance from center
		int dist = abs(x - centerX);
		// Two expanding waves with phase lag
		uint8_t wave1 = sin8((uint8_t)(t + (dist * 4)));
		uint8_t wave2 = sin8((uint8_t)(tPhase + (dist * 4)));
		uint8_t combined = qadd8(scale8(wave1, 200), scale8(wave2, 100));
		uint8_t vOut = scale8(ANIMATION_DEFAULT_VAL, combined);

		for (int y = 0; y < hgt; ++y) {
			uint8_t hOut = (uint8_t)(animCfg.hue + (y == 0 ? 0 : 16));  // slight hue shift per row
			matrix->setPixelHSV(x, y, hOut, animCfg.sat, vOut);
		}
	}

	// show() is managed by AppController
}

// Base class provides ISerializable
