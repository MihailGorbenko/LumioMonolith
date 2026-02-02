#include "RainbowChaseAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>

RainbowChaseAnimation::RainbowChaseAnimation(uint16_t id, LedMatrix* m)
	: AnimationBase(RAINBOWCHASE_DEFAULT_HUE, id, m) {}

void RainbowChaseAnimation::render() {
	LedMatrix* m = matrix;
	if (!m) return;
	m->clear();

	int w = m->getWidth();
	int hgt = m->getHeight();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	const uint8_t speed = 6; // higher = faster
	uint8_t t = (uint8_t)((millis() / speed) & 0xFF);

	// Moving rainbow, slight row offset for 2-row matrices
	for (int x = 0; x < w; ++x) {
		uint8_t xHue = (uint8_t)(animCfg.hue + t + (uint8_t)((x * 256) / max(1, w)));
		for (int y = 0; y < hgt; ++y) {
			uint8_t rowShift = (uint8_t)(y * 24);
			m->setPixelHSV(x, y, (uint8_t)(xHue + rowShift), ANIMATION_DEFAULT_SAT, ANIMATION_DEFAULT_VAL);
		}
	}


}
