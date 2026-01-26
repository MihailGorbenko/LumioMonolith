#include "PlasmaAnimation.hpp"
#include <FastLED.h>

PlasmaAnimation::PlasmaAnimation(LedMatrix& m)
	: AnimationBase(m, PLASMA_DEFAULT_HUE, PLASMA_DEFAULT_SAT, PLASMA_DEFAULT_VAL) {
    name = PLASMA_ANIMATION_NAME;
}

void PlasmaAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int hgt = matrix->height();
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
			uint8_t hOut = (uint8_t)(hue + (t32 >> 3) + (p >> 2));
			// allow full brightness at peaks; keep small floor of 16
			uint8_t vOut = scale8(val, qadd8(16, p));
			matrix->setPixelHSV(x, y, hOut, sat, vOut);
		}
	}

	matrix->show();
}

bool PlasmaAnimation::serialize(uint8_t* out, size_t maxLen) const {
	if (!out || maxLen < 2) return false;
	out[0] = hue;
	out[1] = sat;
	return true;
}

bool PlasmaAnimation::deserialize(const uint8_t* data, size_t len) {
	if (!data || len < 2) return false;
	setColorHSV(data[0], data[1], ANIMATION_DEFAULT_VAL);
	return true;
}
