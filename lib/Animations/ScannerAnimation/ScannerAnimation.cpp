#include "ScannerAnimation.hpp"
#include <FastLED.h>

ScannerAnimation::ScannerAnimation(LedMatrix& m)
	: AnimationBase(m, SCANNER_DEFAULT_HUE, SCANNER_DEFAULT_SAT, SCANNER_DEFAULT_VAL),
	  tail(255) {
}

void ScannerAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
	// Scanner: hue is the main color, sat and val control appearance
	AnimationBase::setColorHSV(h, s, v);
}

void ScannerAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 1) w = 2;
	if (hgt <= 0) hgt = 1;

	// Ping-pong position (triangle wave)
	const uint16_t periodMs = 18; // smaller = faster
	uint32_t step = millis() / periodMs;
	uint32_t span = (uint32_t)(w - 1);
	uint32_t phase = step % (2 * span);
	int head = (phase <= span) ? (int)phase : (int)(2 * span - phase);

	for (int x = 0; x < w; ++x) {
		int dist = abs(x - head);
		// Tail falloff: dist 0 => 255, larger dist => smaller
		uint8_t fall = qsub8(255, (uint8_t)min(255, dist * 32));
		uint8_t vOut = scale8(val, scale8(fall, tail));
		if (vOut == 0) continue;

		// Alternate rows for extra motion on 2-row matrices
		for (int y = 0; y < hgt; ++y) {
			uint8_t rowMask = (hgt >= 2) ? (uint8_t)((y == (head & 1)) ? 255 : 170) : 255;
			matrix->setPixelHSV(x, y, hue, sat, scale8(vOut, rowMask));
		}
	}

	matrix->show();
}
