#include "RainbowChaseAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>

RainbowChaseAnimation::RainbowChaseAnimation(uint16_t id, LedMatrix& m)
	: AnimationBase(RAINBOWCHASE_DEFAULT_HUE, id, m), cachedWidth(0), cachedHeight(0) {}

void RainbowChaseAnimation::onActivate() {
	LedMatrix& m = matrix;
	int w = m.getWidth();
	int h = m.getHeight();
	if (w <= 0) w = 1;
	if (h <= 0) h = 1;
	const int MAX_DIM = 256;
	if (w > MAX_DIM) w = MAX_DIM;
	if (h > MAX_DIM) h = MAX_DIM;

	cachedWidth = w;
	cachedHeight = h;

	colHueOffset.clear(); colHueOffset.resize(w);
	rowShift.clear(); rowShift.resize(h);

	for (int x = 0; x < w; ++x) {
		colHueOffset[x] = (uint8_t)((x * 256) / (w > 0 ? w : 1));
	}
	for (int y = 0; y < h; ++y) {
		rowShift[y] = (uint8_t)(y * 24);
	}

	AnimationBase::onActivate();
}

void RainbowChaseAnimation::render() {
	if (!isInitialized()) return;
	LedMatrix& m = matrix;
	m.clear();

	const int w = cachedWidth;
	const int hgt = cachedHeight;
	if (w <= 0 || hgt <= 0) return;

	const uint8_t speed = 6; // higher = faster
	uint8_t t = (uint8_t)((millis() / speed) & 0xFF);

	for (int x = 0; x < w; ++x) {
		uint8_t xHue = (uint8_t)(animCfg.hue + t + colHueOffset[x]);
		for (int y = 0; y < hgt; ++y) {
			m.setPixelHSV(x, y, (uint8_t)(xHue + rowShift[y]), ANIMATION_DEFAULT_SAT, ANIMATION_DEFAULT_VAL);
		}
	}

}
