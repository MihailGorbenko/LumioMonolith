#include "NeonGridAnimation.hpp"

NeonGridAnimation::NeonGridAnimation(LedMatrix& m)
	: AnimationBase(m, NEONGRID_DEFAULT_HUE, NEONGRID_DEFAULT_SAT, NEONGRID_DEFAULT_VAL) {
	memset(gridPulse, 0, sizeof(gridPulse));
}

void NeonGridAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
	// Neon grid: cyan/magenta/white retro style
	AnimationBase::setColorHSV(h, s, v);
}

void NeonGridAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;
	w = min(w, MAX_W);

	uint16_t t = (uint16_t)(millis() / 8);

	// Horizontal grid lines (constant)
	for (int x = 0; x < w; ++x) {
		uint8_t baseIntensity = 255; // allow full brightness
		for (int y = 0; y < hgt; ++y) {
			uint8_t vOut = scale8(val, baseIntensity);
			uint8_t hOut = (uint8_t)(hue + (y * 64));  // hue per row
			matrix->setPixelHSV(x, y, hOut, sat, vOut);
		}
	}

	// Vertical pulse "grid nodes" (moving wave)
	for (int x = 0; x < w; ++x) {
		uint8_t pulseWave = sin8((uint8_t)(t + x * 6));
		uint8_t intensity = scale8(pulseWave, val);

		for (int y = 0; y < hgt; ++y) {
			uint8_t hOut = (uint8_t)(hue + intensity / 2);
			uint8_t vOut = qadd8(scale8(val, 255), scale8(val, scale8(pulseWave, 255)));
			vOut = min(vOut, val);
			matrix->setPixelHSV(x, y, hOut, sat, vOut);
		}
	}

	matrix->show();
}
