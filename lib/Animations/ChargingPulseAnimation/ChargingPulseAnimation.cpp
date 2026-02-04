#include "ChargingPulseAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>
#include <math.h>

static inline float easeInOutCubic(float p) {
    if (p <= 0.0f) return 0.0f;
    if (p >= 1.0f) return 1.0f;
    if (p < 0.5f) {
        return 4.0f * p * p * p;
    } else {
        float f = (-2.0f * p + 2.0f);
        return 1.0f - (f * f * f) / 2.0f;
    }
}

ChargingPulseAnimation::ChargingPulseAnimation(uint16_t id, LedMatrix& m)
    : AnimationBase(CHARGING_DEFAULT_HUE, id, m) {}

void ChargingPulseAnimation::onActivate() {
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
    cachedAscendMs = h * CHARGING_RING_STEP_MS;
    cachedCycleMs = cachedAscendMs + CHARGING_FLASH_MS + CHARGING_FADE_MS;
    invRingStep = (CHARGING_RING_STEP_MS == 0) ? 0.0f : (1.0f / (float)CHARGING_RING_STEP_MS);
    invAscend = (cachedAscendMs > 0) ? (1.0f / (float)cachedAscendMs) : 0.0f;

    startMs.assign(h, 0);
    for (int y = 0; y < h; ++y) {
        startMs[y] = (h - 1 - y) * CHARGING_RING_STEP_MS;
    }

    AnimationBase::onActivate();
}

void ChargingPulseAnimation::render() {
    if (!isInitialized()) return;
    LedMatrix& m = matrix;

    const int w = cachedWidth;
    const int h = cachedHeight;
    if (w <= 0 || h <= 0) return;

    const uint32_t now = millis();
    const int cycleMsSafe = (cachedCycleMs > 0) ? cachedCycleMs : 1;
    const int t = (int)(now % (uint32_t)cycleMsSafe);

    m.clear();

    const bool inAscend = (t < cachedAscendMs);
    const bool inFlash  = (!inAscend && t < (cachedAscendMs + CHARGING_FLASH_MS));
    const bool inFade   = (!inAscend && !inFlash);

    float globalRamp = 1.0f;
    if (inAscend) {
        globalRamp = easeInOutCubic((float)t * invAscend); // 0..1
    }

    uint8_t vTopFlash = ANIMATION_DEFAULT_VAL;
    if (inFlash) {
        int tf = t - cachedAscendMs; // 0..FLASH
        uint8_t f256 = (uint8_t)((uint32_t)tf * 255U / (uint32_t)CHARGING_FLASH_MS);
        uint8_t s = sin8(f256);
        uint16_t add = ((uint16_t)(255 - ANIMATION_DEFAULT_VAL) * (uint16_t)s) / 255U;
        vTopFlash = (uint8_t)min(255, (int)ANIMATION_DEFAULT_VAL + (int)add);
    }

    for (int y = 0; y < h; ++y) {
        uint8_t vOut = 0;

        if (inAscend) {
            int sMs = startMs[y];
            if (t >= sMs) {
                float u = (float)(t - sMs) * invRingStep; // 0..1
                if (u < 1.0f) {
                    float e = easeInOutCubic(u);
                    float v = (float)ANIMATION_DEFAULT_VAL * e * globalRamp;
                    if (v > 255.0f) v = 255.0f;
                    vOut = (uint8_t)v;
                } else {
                    float v = (float)ANIMATION_DEFAULT_VAL * globalRamp;
                    if (v > 255.0f) v = 255.0f;
                    vOut = (uint8_t)v;
                }
            } else {
                vOut = 0;
            }
        } else if (inFlash) {
            vOut = ANIMATION_DEFAULT_VAL;
            if (y == 0) vOut = vTopFlash;
        } else if (inFade) {
            int tf = t - (cachedAscendMs + CHARGING_FLASH_MS);
            float u = (float)tf / (float)CHARGING_FADE_MS;
            float e = easeInOutCubic(u);
            float f = 1.0f - e;
            vOut = (uint8_t)((float)ANIMATION_DEFAULT_VAL * f);
        }

        if (vOut > 0) {
            for (int x = 0; x < w; ++x) {
                m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vOut);
            }
        }
    }

}

// Base class provides ISerializable
