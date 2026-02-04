#include "PulseWaveAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>


PulseWaveAnimation::PulseWaveAnimation(uint16_t id, LedMatrix& m)
		: AnimationBase(PULSEWAVE_DEFAULT_HUE, id, m),
			pulseRadius(0), cachedWidth(0), cachedHeight(0), centerX(0) {}

void PulseWaveAnimation::onActivate() {
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
	centerX = w / 2;

	colDist4.clear(); colDist4.resize(w);
	rowHueShift.clear(); rowHueShift.resize(h);

	for (int x = 0; x < w; ++x) {
		int dist = abs(x - centerX);
		colDist4[x] = (uint8_t)((dist * 4) & 0xFF); // modulo 256
	}
	for (int y = 0; y < h; ++y) {
		rowHueShift[y] = (uint8_t)(y == 0 ? 0 : 16);
	}

	AnimationBase::onActivate();
}

void PulseWaveAnimation::render() {
	if (!isInitialized()) return;
	LedMatrix& m = matrix;
	m.clear();

	const int w = cachedWidth;
	const int hgt = cachedHeight;
	if (w <= 0 || hgt <= 0) return;

	// Cosmic pulse expanding from center
	uint32_t now = millis();
	uint32_t t32 = now / 6U;
	uint8_t t = (uint8_t)(t32 & 0xFF);
	uint8_t tPhase = (uint8_t)((t32 + 128U) & 0xFF);

	for (int x = 0; x < w; ++x) {
		uint8_t dist4 = colDist4[x];
		uint8_t wave1 = sin8((uint8_t)(t + dist4));
		uint8_t wave2 = sin8((uint8_t)(tPhase + dist4));
		uint8_t combined = qadd8(scale8(wave1, 200), scale8(wave2, 100));
		uint8_t vOut = scale8(ANIMATION_DEFAULT_VAL, combined);

		for (int y = 0; y < hgt; ++y) {
			uint8_t hOut = (uint8_t)(animCfg.hue + rowHueShift[y]);
			m.setPixelHSV(x, y, hOut, ANIMATION_DEFAULT_SAT, vOut);
		}
	}

}

// Base class provides ISerializable
