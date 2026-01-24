#include "CenterPulseAnimation.hpp"
#include <FastLED.h>

CenterPulseAnimation::CenterPulseAnimation(LedMatrix& m)
    : AnimationBase(m, CENTERPULSE_DEFAULT_HUE, CENTERPULSE_DEFAULT_SAT, CENTERPULSE_DEFAULT_VAL),
      stride(72), speedDiv(6) {}

void CenterPulseAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
    AnimationBase::setColorHSV(h, s, v);
}

void CenterPulseAnimation::setStride(uint8_t s) { stride = (s == 0) ? 1 : s; }
void CenterPulseAnimation::setSpeedDiv(uint8_t div) { speedDiv = (div == 0) ? 1 : div; }

void CenterPulseAnimation::render() {
    if (!matrix) return;
    int w = matrix->width();
    int h = matrix->height();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    // Center row index (works for odd/even heights)
    int center = (h - 1) / 2;
    // Max radius to edges from center (supports odd/even heights)
    int maxR = max(center, (h - 1) - center);
    if (maxR < 0) maxR = 0;

    // Compute expanding radius 0..maxR..0 using sin wave
    uint8_t phase = (uint8_t)(millis() / speedDiv);
    uint8_t s = sin8(phase); // 0..255
    uint16_t sr = (uint16_t)s * (uint16_t)maxR; // 0..maxR*255
    int radius = (int)(sr / 255);
    uint8_t frac = (uint8_t)(sr % 255); // fractional part for edge blend

    matrix->clear();
    for (int y = 0; y < h; ++y) {
        int d = abs(y - center);
        uint8_t vRow = 0;
        if (d < radius) {
            vRow = val; // fully lit inside the current radius
        } else if (d == radius) {
            // inside boundary row remains fully lit as radius grows
            vRow = val;
        } else if (d == (radius + 1)) {
            // frontier row beyond current radius: blend in progressively
            vRow = scale8(val, frac);
        } else {
            vRow = 0;
        }
        if (!vRow) continue;
        for (int x = 0; x < w; ++x) {
            matrix->setPixelHSV(x, y, hue, sat, vRow);
        }
    }
    matrix->show();
}
