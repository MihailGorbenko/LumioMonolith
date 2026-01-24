#include "GalacticWarpAnimation.hpp"
#include <FastLED.h>

GalacticWarpAnimation::GalacticWarpAnimation(LedMatrix& m)
	: AnimationBase(m, GALACTICWARP_DEFAULT_HUE, GALACTICWARP_DEFAULT_SAT, GALACTICWARP_DEFAULT_VAL) {
}

void GalacticWarpAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
	// Galactic warp: cosmic colours, expanding spirals
	AnimationBase::setColorHSV(h, s, v);
}

void GalacticWarpAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	uint16_t t = (uint16_t)(millis() / 5);
	uint8_t centerX = w / 2;

	for (int x = 0; x < w; ++x) {
		// Distance-based warping effect: spiral/tunnel
		int dist = abs(x - (int)centerX);
		// Three interlocking waves creating warp tunnel effect
		uint8_t wave1 = sin8((uint8_t)(t + dist * 8));
		uint8_t wave2 = sin8((uint8_t)(t - dist * 6 + 85));
		uint8_t wave3 = sin8((uint8_t)(t + dist * 4 + 170));

		// Combine waves with different priorities
		uint8_t intensity = qadd8(wave1, scale8(qadd8(wave2, wave3), 128));

		for (int y = 0; y < hgt; ++y) {
			// Spiral effect: hue shifts with distance and time
			uint8_t spiralHue = (uint8_t)(hue + t / 2 + dist * 4 + y * 64);
			uint8_t vOut = scale8(val, intensity);
			matrix->setPixelHSV(x, y, spiralHue, sat, vOut);
		}
	}

	// Add central "bright core" (hyperspace center)
	int coreX = (int)centerX;
	for (int y = 0; y < hgt; ++y) {
		uint8_t coreBrightness = 255 - (uint8_t)((millis() / 3) % 256);
		uint8_t hOut = (uint8_t)(hue + (t & 0xFF) / 2);
		matrix->setPixelHSV(coreX, y, hOut, sat, coreBrightness);
	}

	matrix->show();
}
