#include "StarsAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>

StarsAnimation::StarsAnimation(uint16_t id, LedMatrix& m)
	: AnimationBase(STARS_DEFAULT_HUE, id, m) {
    // defer allocation to first render when matrix size is known
    starCount = 0;
}

// Base class provides ISerializable

void StarsAnimation::randomizeStar(Star& s, int w, int h) {
    if (w <= 0) w = 8;
    if (h <= 0) h = 8;
	s.x = random(0, (uint8_t)w);
	s.y = random(0, (uint8_t)h);
	s.brightness = random(0, 256);
	s.target = random(0, 256);
	s.nextChangeMillis = millis() + random(400, 2200); // slower target change
	// Parallax: depth, initial fixed-point position, speed
	s.depth = (uint8_t)random(0, 3);
	s.xfp = ((int32_t)s.x) << 8;
	// slower lateral movement
	int8_t baseV = (s.depth == 0) ? -8 : (s.depth == 1) ? -14 : -24;
	if ((uint8_t)random(0, 2)) baseV = -baseV;
	s.vfp = baseV;
	// Smooth twinkle: phase and speed
	s.twPhase = (uint8_t)random(0, 256);
	s.twSpeed = (uint8_t)((s.depth == 0) ? 1 : (s.depth == 1) ? 1 : 2);
}

void StarsAnimation::onActivate() {
	// Prepare star field when the animation is activated to avoid doing
	// allocation and initialization during the render loop.
	LedMatrix& m = matrix;
	int w = m.getWidth();
	int h = m.getHeight();
	if (w <= 0) w = 8;
	if (h <= 0) h = 8;

	if (stars.empty()) {
		int total_pixels = w * h;
		int autoCount = max(1, (total_pixels * 3) / 5);
		starCount = autoCount;
		stars.reserve(starCount);
		int base = min(w, starCount);
		for (int i = 0; i < base; ++i) {
			Star s;
			s.x = (uint8_t)((long)i * w / base);
			s.y = (uint8_t)random(0, h);
			s.brightness = (uint8_t)random(0, 256);
			s.target = (uint8_t)random(0, 256);
			s.nextChangeMillis = millis() + (unsigned long)random(400, 2200);
			s.depth = (uint8_t)random(0, 3);
			s.xfp = ((int32_t)s.x) << 8;
			int8_t baseV = (s.depth == 0) ? -8 : (s.depth == 1) ? -14 : -24;
			if ((uint8_t)random(0, 2)) baseV = -baseV;
			s.vfp = baseV;
			s.twPhase = (uint8_t)random(0, 256);
			s.twSpeed = (uint8_t)((s.depth == 0) ? 1 : (s.depth == 1) ? 1 : 2);
			stars.push_back(s);
		}
		for (int i = base; i < starCount; ++i) {
			Star s;
			randomizeStar(s, w, h);
			stars.push_back(s);
		}
	}

	// Cache matrix size for faster render
	cachedWidth = w;
	cachedHeight = h;
	lastMillis = millis();
	AnimationBase::onActivate();
}

void StarsAnimation::render() {
	LedMatrix& m = matrix;
	uint32_t now = millis();
	// clear matrix before drawing (controller sets render cadence)
	m.clear();

	// dimensions for wrapping and fixed-point math
	// Use cached dimensions populated in onActivate(); assume prepared.
	int w = cachedWidth;
	int h = cachedHeight;
	if (w <= 0) w = 8; // defensive clamp in case onActivate wasn't called
	if (h <= 0) h = 8;
	int w8 = w << 8;

	// stars vector is initialized in onActivate(); assume prepared here.

	// dt in milliseconds with a first-frame safeguard
	uint32_t dt = 0;
	if (lastMillis == 0) dt = 16;
	else dt = now - lastMillis;
	// Clamp dt to a reasonable maximum to avoid large jumps or integer overflow
	if (dt > 1000U) dt = 1000U;

	for (auto &s : stars) {
		// Update target brightness using sin8 + noise8 for natural variation
		// noise per-star gives spatial variance
		uint8_t noise = inoise8((uint16_t)s.x * 17u + (uint16_t)s.y * 29u);
		// tw is 0..255
		uint8_t tw = sin8((uint8_t)(s.twPhase) + noise);
		// scale tw by master val to get a target in 0..val
		uint8_t computedTarget = scale8(tw, (uint8_t)ANIMATION_DEFAULT_VAL);
		// occasionally refresh target timing
		if (now >= s.nextChangeMillis) {
			s.target = computedTarget;
			s.nextChangeMillis = now + (unsigned long)random(100, 1500);
		}

		// Update X position considering parallax and dt (fixed-point)
		// slower movement: increase divisor so per-ms movement is smaller
		int32_t delta = ((int32_t)s.vfp * (int32_t)dt) / 64; // smoother, slower motion
		s.xfp = (int32_t)(s.xfp + delta);
		// Proper wrap within 0..w8-1
		while (s.xfp >= w8) s.xfp -= w8;
		while (s.xfp < 0) s.xfp += w8;
		s.x = (uint8_t)(s.xfp >> 8);

		// Update twinkle (brightness -> target) in small dt-dependent steps
		if (s.brightness < s.target) {
			uint16_t diff = (uint16_t)(s.target - s.brightness);
			// slower smoothing: larger divisor for dt
			uint16_t step = (dt / 200u) + 1u; // much slower approach
			if (step > diff) step = diff;
			s.brightness = (uint8_t)(s.brightness + (uint8_t)step);
		} else if (s.brightness > s.target) {
			uint16_t diff = (uint16_t)(s.brightness - s.target);
			uint16_t step = (dt / 200u) + 1u;
			if (step > diff) step = diff;
			s.brightness = (uint8_t)(s.brightness - (uint8_t)step);
		}

		// advance twinkle phase
		s.twPhase = (uint8_t)(s.twPhase + s.twSpeed);

		// Make stars more visible: use linear brightness and apply a lower threshold
		uint8_t baseV = s.brightness;
		uint8_t depthGain = (s.depth == 0) ? 180 : (s.depth == 1) ? 220 : 255;
		baseV = scale8(baseV, depthGain);
		// soften with tw (already used for target) — apply small modulation
		// milder twinkle modulation (slower/smoother)
		uint8_t twMod = (uint8_t)(160 + (tw >> 3));
		baseV = scale8(baseV, twMod);
		uint8_t drawV = scale8((uint8_t)ANIMATION_DEFAULT_VAL, baseV);
		if (drawV < 12) continue; // cull very dim stars
		// slight hue shift by depth for parallax color
		uint8_t starHue = (uint8_t)(animCfg.hue + (s.depth == 2 ? 0 : (s.depth == 1 ? 4 : 8)));
	m.setPixelHSV(s.x, s.y, starHue, ANIMATION_DEFAULT_SAT, drawV);
	}

	lastMillis = now;

}
