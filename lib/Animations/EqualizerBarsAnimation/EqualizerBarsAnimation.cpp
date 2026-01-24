#include "EqualizerBarsAnimation.hpp"
#include <FastLED.h>

EqualizerBarsAnimation::EqualizerBarsAnimation(LedMatrix& m)
        : AnimationBase(m, EQ_DEFAULT_HUE, EQ_DEFAULT_SAT, EQ_DEFAULT_VAL),
            speedDiv(3), jitter(80), step(1), nextStepMs(0), stepPeriodMs(50) {
        int w = m.getWidth();
        int h = m.getHeight();
        for (int x = 0; x < w; ++x) {
                heights[x] = random8(0, h + 1);
                velocity[x] = (int8_t)((int8_t)random8(0, 3) - 1); // -1..+1
        }
}

void EqualizerBarsAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
    AnimationBase::setColorHSV(h, s, v);
}

void EqualizerBarsAnimation::setSpeedDiv(uint8_t div) { speedDiv = (div == 0) ? 1 : div; }
void EqualizerBarsAnimation::setJitter(uint8_t j) { jitter = j; }

void EqualizerBarsAnimation::render() {
    if (!matrix) return;
    int w = matrix->width();
    int h = matrix->height();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    uint32_t now = millis();
    if (now >= nextStepMs) {
        nextStepMs = now + (stepPeriodMs * speedDiv);
        // continuous per-column motion with slight random acceleration
        for (int x = 0; x < w; ++x) {
            int8_t dv = (int8_t)((int8_t)random8(0, 3) - 1); // -1..+1
            velocity[x] = (int8_t)constrain((int)velocity[x] + dv, -2, 2);
            int nh = (int)heights[x] + (int)velocity[x] * (int)step;
            if (nh < 0) { nh = 0; velocity[x] = (int8_t)abs(velocity[x]); }
            if (nh > h) { nh = h; velocity[x] = (int8_t)(-abs(velocity[x])); }
            heights[x] = (uint8_t)nh;
        }
    }

    matrix->clear();
    // draw columns from bottom
    for (int x = 0; x < w; ++x) {
        uint8_t colHeight = heights[x];
        for (int y = 0; y < h; ++y) {
            bool lit = (y >= (h - colHeight)); // bottom-up
            if (lit) {
                // brightness gradient up the bar
                uint8_t pos = (uint8_t)(y - (h - colHeight)); // 0..colHeight-1
                uint8_t b = scale8(val, 200 - scale8(pos * 255 / max<uint8_t>(1, colHeight), 160));
                matrix->setPixelHSV(x, y, hue, sat, b);
            }
        }
    }

    matrix->show();
}
