#include "PlasmaAnimation.hpp"
#include <FastLED.h>

PlasmaAnimation::PlasmaAnimation(LedMatrix& m)
	: AnimationBase(m, PLASMA_DEFAULT_HUE, PLASMA_DEFAULT_SAT, PLASMA_DEFAULT_VAL) {
}

void PlasmaAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
	// Plasma: hue drifts dynamically, but base hue is stored and used as offset
	AnimationBase::setColorHSV(h, s, v);
}

void PlasmaAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	uint16_t t = (uint16_t)(millis() / 4);

	for (int x = 0; x < w; ++x) {
		uint8_t sx = sin8((uint8_t)(x * 8 + (t & 0xFF)));
		for (int y = 0; y < hgt; ++y) {
			uint8_t sy = sin8((uint8_t)(y * 48 + ((t >> 1) & 0xFF)));
			uint8_t p = (uint8_t)((sx + sy) >> 1);

			// Hue slowly drifts, value is modulated by plasma field
			uint8_t hOut = (uint8_t)(hue + (t >> 3) + (p >> 2));
			// allow full brightness at peaks; keep small floor of 16
			uint8_t vOut = scale8(val, qadd8(16, p));
			matrix->setPixelHSV(x, y, hOut, sat, vOut);
		}
	}

	matrix->show();
}
