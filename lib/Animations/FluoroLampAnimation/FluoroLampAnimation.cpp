#include "FluoroLampAnimation.hpp"
#include <FastLED.h>

FluoroLampAnimation::FluoroLampAnimation(LedMatrix& m)
	: AnimationBase(m, FLUOROLAMP_DEFAULT_HUE, FLUOROLAMP_DEFAULT_SAT, FLUOROLAMP_DEFAULT_VAL),
	  startedAt(millis()), warmupMs(1400), flickerAmt(22),
	  blinkUntilMs(0), nextBlinkAtMs(0), blinkStartX(0), blinkLen(0) {
}

void FluoroLampAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
	AnimationBase::setColorHSV(h, s, v);
}

void FluoroLampAnimation::setWarmupMs(uint16_t ms) { warmupMs = ms; }
void FluoroLampAnimation::setFlickerStrength(uint8_t amt) { flickerAmt = amt; }

void FluoroLampAnimation::reset() {
	startedAt = millis();
	blinkUntilMs = 0;
	nextBlinkAtMs = 0;
}

void FluoroLampAnimation::scheduleBlink(int w) {
	if (w <= 0) return;
	blinkStartX = (uint8_t)random(0, max(1, w - 1));
	blinkLen = (uint8_t)random(1, min(5, w - blinkStartX));
	blinkUntilMs = millis() + (unsigned long)random(30, 120); // very short flash
}

void FluoroLampAnimation::render() {
	if (!matrix) return;
	unsigned long now = millis();

	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	bool inWarmup = (unsigned long)(now - startedAt) < warmupMs;
	uint8_t base = val;

	// Warmup ramp from dim to full
	if (inWarmup) {
		unsigned long e = (now - startedAt);
		if (e > warmupMs) e = warmupMs;
		uint8_t ramp = (uint8_t)((e * 255UL) / max(1UL, (unsigned long)warmupMs));
		// Start from a dim level to simulate tube strike
		base = qadd8(24, scale8(val, ramp));

		// Random quick blinks in segments along the tube
		if (now >= nextBlinkAtMs) {
			nextBlinkAtMs = now + (unsigned long)random(60, 180);
			scheduleBlink(w);
		}
	}

	matrix->clear();

	// Continuous subtle flicker (post-warmup too)
	// Use phase per column for spatial variation
	for (int x = 0; x < w; ++x) {
		// Distance-to-ends falloff: ends slightly dimmer than center
		int d = min(x, w - 1 - x);
		uint8_t endBoost = (uint8_t)constrain(180 + (d * 8), 180, 255); // 180..255

		// Mains-like flicker: small sinus modulation + pseudo-noise
		uint8_t s1 = sin8((uint8_t)((now >> 1) + x * 3));
		uint8_t s2 = sin8((uint8_t)((now >> 3) + x * 7 + 91));
		uint8_t flicker = scale8(s1, flickerAmt) + scale8(s2, (uint8_t)(flickerAmt / 2));
		uint8_t vCol = qadd8(base, flicker);
		vCol = scale8(vCol, endBoost);

		// Warmup segment flashes: brief local boosts
		if (inWarmup && now < blinkUntilMs) {
			if (x >= blinkStartX && x < (int)(blinkStartX + blinkLen)) {
				vCol = qadd8(vCol, (uint8_t)random(40, 120));
			}
		}

		for (int y = 0; y < hgt; ++y) {
			// small vertical variation between rows, if present
			uint8_t rowV = (y == 0 ? vCol : scale8(vCol, 235));
			matrix->setPixelHSV(x, y, hue, sat, rowV);
		}
	}

	matrix->show();
}
