#include "PlasmaAnimation.hpp"
#include <FastLED.h>

PlasmaAnimation::PlasmaAnimation(uint16_t id)
	: AnimationBase(PLASMA_DEFAULT_HUE, id) {}

void PlasmaAnimation::render(LedMatrix& m) {
	m.clear();

	int w = m.getWidth();
	int hgt = m.getHeight();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	uint32_t t32 = millis() / 4U;
	uint8_t t = (uint8_t)(t32 & 0xFF);
	uint8_t t1 = (uint8_t)((t32 >> 1) & 0xFF);

	for (int x = 0; x < w; ++x) {
		uint8_t sx = sin8((uint8_t)(x * 8 + t));
		for (int y = 0; y < hgt; ++y) {
			uint8_t sy = sin8((uint8_t)(y * 48 + t1));
			uint8_t p = (uint8_t)((sx + sy) >> 1);

			// Hue slowly drifts, value is modulated by plasma field
			uint8_t hOut = (uint8_t)(animCfg.hue + (t32 >> 3) + (p >> 2));
			// allow full brightness at peaks; keep small floor of 16
			uint8_t vOut = scale8(ANIMATION_DEFAULT_VAL, qadd8(16, p));
			m.setPixelHSV(x, y, hOut, ANIMATION_DEFAULT_SAT, vOut);
		}
	}

	// show() is managed by AppManager
}
