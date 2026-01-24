#include "PulseWaveAnimation.hpp"
#include <FastLED.h>

PulseWaveAnimation::PulseWaveAnimation(LedMatrix& m)
	: AnimationBase(m, PULSEWAVE_DEFAULT_HUE, PULSEWAVE_DEFAULT_SAT, PULSEWAVE_DEFAULT_VAL),
	  pulseRadius(0) {
}

void PulseWaveAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
	// Sci-Fi pulse: hue is core color, sat/val control intensity
	AnimationBase::setColorHSV(h, s, v);
}

void PulseWaveAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	// Cosmic pulse expanding from center
	uint16_t t = (uint16_t)(millis() / 6);
	uint8_t centerX = w / 2;
	
	for (int x = 0; x < w; ++x) {
		// Distance from center
		int dist = abs(x - (int)centerX);
		// Two expanding waves with phase lag
		uint8_t wave1 = sin8((uint8_t)t + (dist * 4));
		uint8_t wave2 = sin8((uint8_t)(t + 128) + (dist * 4));
		uint8_t combined = qadd8(scale8(wave1, 200), scale8(wave2, 100));
		
		for (int y = 0; y < hgt; ++y) {
			uint8_t vOut = scale8(val, combined);
			uint8_t hOut = (uint8_t)(hue + (y == 0 ? 0 : 16));  // slight hue shift per row
			matrix->setPixelHSV(x, y, hOut, sat, vOut);
		}
	}

	matrix->show();
}
