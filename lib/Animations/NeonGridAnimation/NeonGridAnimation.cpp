#include "NeonGridAnimation.hpp"

NeonGridAnimation::NeonGridAnimation(LedMatrix& m)
	: AnimationBase(m, NEONGRID_DEFAULT_HUE, NEONGRID_DEFAULT_SAT, NEONGRID_DEFAULT_VAL) {
}

void NeonGridAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;
	w = min(w, MAX_W);

	uint32_t now = millis();
	uint32_t t32 = now / 8U;
	uint8_t t = (uint8_t)(t32 & 0xFF);

	// Horizontal grid lines (base color per row)
	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < hgt; ++y) {
			uint8_t vOut = val; // base full brightness
			uint8_t hOut = (uint8_t)(hue + (y * 64));  // hue per row
			matrix->setPixelHSV(x, y, hOut, sat, vOut);
		}
	}

	// Vertical pulse "grid nodes" (moving wave) - actually modulate brightness up to val
	for (int x = 0; x < w; ++x) {
		uint8_t pulseWave = sin8((uint8_t)(t + x * 6));
		// base is a bit dimmer so pulse is visible; then clamp to val
		uint8_t base = scale8(val, 200); // ~78% of val
		uint8_t add = scale8(val, pulseWave); // pulse contribution
		uint8_t vOut = min((uint8_t)val, qadd8(base, add));

		for (int y = 0; y < hgt; ++y) {
			uint8_t hOut = (uint8_t)(hue + (vOut >> 3));
			matrix->setPixelHSV(x, y, hOut, sat, vOut);
		}
	}

	matrix->show();
}
