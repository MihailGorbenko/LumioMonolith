#include "RainbowChaseAnimation.hpp"
#include <FastLED.h>

RainbowChaseAnimation::RainbowChaseAnimation(LedMatrix& m)
	: AnimationBase(m, RAINBOWCHASE_DEFAULT_HUE, RAINBOWCHASE_DEFAULT_SAT, RAINBOWCHASE_DEFAULT_VAL) {
}

void RainbowChaseAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
	// Rainbow: hue offset, saturation and value control intensity
	AnimationBase::setColorHSV(h, s, v);
}

void RainbowChaseAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	const uint8_t speed = 6; // higher = faster
	uint8_t t = (uint8_t)((millis() / speed) & 0xFF);

	// Moving rainbow, slight row offset for 2-row matrices
	for (int x = 0; x < w; ++x) {
		uint8_t xHue = (uint8_t)(hue + t + (uint8_t)((x * 256) / max(1, w)));
		for (int y = 0; y < hgt; ++y) {
			uint8_t rowShift = (uint8_t)(y * 24);
			matrix->setPixelHSV(x, y, (uint8_t)(xHue + rowShift), sat, val);
		}
	}

	matrix->show();
}
