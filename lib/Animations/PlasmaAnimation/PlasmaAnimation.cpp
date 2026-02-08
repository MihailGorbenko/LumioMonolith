#include "PlasmaAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>

PlasmaAnimation::PlasmaAnimation(uint16_t id, LedMatrix& m)
	: AnimationBase(PLASMA_DEFAULT_HUE, id, m) {}

void PlasmaAnimation::onActivate() {
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

	colBase.clear(); colBase.resize(w);
	rowBase.clear(); rowBase.resize(h);

	for (int x = 0; x < w; ++x) {
		colBase[x] = (uint8_t)(x * 8);
	}
	for (int y = 0; y < h; ++y) {
		rowBase[y] = (uint8_t)(y * 48);
	}

	AnimationBase::onActivate();
}

void PlasmaAnimation::render() {
	if (!isInitialized()) return;
	LedMatrix& m = matrix;
	m.clear();

	const int w = cachedWidth;
	const int hgt = cachedHeight;

	uint32_t t32 = millis() / 4U;
	uint8_t t = (uint8_t)(t32 & 0xFF);
	uint8_t t1 = (uint8_t)((t32 >> 1) & 0xFF);

	for (int x = 0; x < w; ++x) {
		uint8_t sx = sin8((uint8_t)(colBase[x] + t));
		for (int y = 0; y < hgt; ++y) {
			uint8_t sy = sin8((uint8_t)(rowBase[y] + t1));
			uint8_t p = (uint8_t)((sx + sy) >> 1);

			// Hue slowly drifts, value is modulated by plasma field
			uint8_t hOut = (uint8_t)(animCfg.hue + (t32 >> 3) + (p >> 2));
			// allow full brightness at peaks; keep small floor of 16
			uint8_t vOut = scale8(ANIMATION_DEFAULT_VAL, qadd8(16, p));
			m.setPixelHSV(x, y, hOut, ANIMATION_DEFAULT_SAT, vOut);
		}
	}

}
