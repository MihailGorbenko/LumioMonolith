#include "ChargingPulseAnimation.hpp"
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

ChargingPulseAnimation::ChargingPulseAnimation(uint16_t id)
    : AnimationBase(CHARGING_DEFAULT_HUE, id) {}

void ChargingPulseAnimation::render(LedMatrix& m) {

    const int w = m.getWidth();
    const int h = m.getHeight();
    if (w <= 0 || h <= 0) return;

    const uint32_t now = millis();

    const int ascendMs = h * CHARGING_RING_STEP_MS;           // bottom -> top ramp total
    const int cycleMs  = ascendMs + CHARGING_FLASH_MS + CHARGING_FADE_MS;
    const int t        = (int)(now % (uint32_t)cycleMs);

    m.clear();

    // Determine phase
    const bool inAscend = (t < ascendMs);
    const bool inFlash  = (!inAscend && t < (ascendMs + CHARGING_FLASH_MS));
    const bool inFade   = (!inAscend && !inFlash);

    // Global brightness ramp during ascent for a stronger "charging" feel
    float globalRamp = 1.0f;
    if (inAscend) {
        globalRamp = easeInOutCubic((float)t / (float)ascendMs); // 0..1
    }

    uint8_t vTopFlash = ANIMATION_DEFAULT_VAL;
    if (inFlash) {
        // Top flash: overshoot to 255 using a smooth sin curve
        int tf = t - ascendMs; // 0..FLASH
        uint8_t f256 = (uint8_t)((uint32_t)tf * 255U / (uint32_t)CHARGING_FLASH_MS);
        uint8_t s = sin8(f256); // 0..255
        // Interpolate from normal val to 255 with sin easing
        uint16_t add = ((uint16_t)(255 - ANIMATION_DEFAULT_VAL) * (uint16_t)s) / 255U;
        vTopFlash = (uint8_t)min(255, (int)ANIMATION_DEFAULT_VAL + (int)add);
    }

    // Compute brightness per ring (row)
    for (int y = 0; y < h; ++y) {
        // bottom row starts first
        int startMs = (h - 1 - y) * CHARGING_RING_STEP_MS;
        uint8_t vOut = 0;

        if (inAscend) {
            if (t >= startMs) {
                float u = (float)(t - startMs) / (float)CHARGING_RING_STEP_MS; // 0..1
                if (u < 1.0f) {
                    float e = easeInOutCubic(u);
                    float v = (float)ANIMATION_DEFAULT_VAL * e * globalRamp; // local + global ramp
                    if (v > 255.0f) v = 255.0f;
                    vOut = (uint8_t)v;
                } else {
                    // fully lit, still scaled by global ramp to keep growing brightness
                    float v = (float)ANIMATION_DEFAULT_VAL * globalRamp;
                    if (v > 255.0f) v = 255.0f;
                    vOut = (uint8_t)v;
                }
            } else {
                vOut = 0;
            }
        } else if (inFlash) {
            // During flash, keep all rings at full brightness; top flashes stronger
            vOut = ANIMATION_DEFAULT_VAL;
            if (y == 0) {
                vOut = vTopFlash;
            }
        } else if (inFade) {
            // Fade all rings to off with easing
            int tf = t - (ascendMs + CHARGING_FLASH_MS);
            float u = (float)tf / (float)CHARGING_FADE_MS; // 0..1
            float e = easeInOutCubic(u);
            float f = 1.0f - e;
            vOut = (uint8_t)((float)ANIMATION_DEFAULT_VAL * f);
        }

        // Draw entire ring with computed brightness
        if (vOut > 0) {
            for (int x = 0; x < w; ++x) {
                m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vOut);
            }
        }
    }

    // show() is managed by AppController
}

// Base class provides ISerializable
