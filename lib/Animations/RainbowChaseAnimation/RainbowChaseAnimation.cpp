#include "RainbowChaseAnimation.hpp"
#include <FastLED.h>

RainbowChaseAnimation::RainbowChaseAnimation(LedMatrix& m)
	: AnimationBase(m, RAINBOWCHASE_DEFAULT_HUE, RAINBOWCHASE_DEFAULT_SAT, RAINBOWCHASE_DEFAULT_VAL) {
    name = RAINBOWCHASE_ANIMATION_NAME;
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

bool RainbowChaseAnimation::serialize(uint8_t* out, size_t maxLen) const {
	if (!out || maxLen < 2) return false;
	out[0] = hue;
	out[1] = sat;
	return true;
}

bool RainbowChaseAnimation::deserialize(const uint8_t* data, size_t len) {
	if (!data || len < 2) return false;
	setColorHSV(data[0], data[1], ANIMATION_DEFAULT_VAL);
	return true;
}
