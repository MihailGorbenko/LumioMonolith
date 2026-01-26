#include "CenterPulseAnimation.hpp"
#include <FastLED.h>

CenterPulseAnimation::CenterPulseAnimation(LedMatrix& m)
        : AnimationBase(m, CENTERPULSE_DEFAULT_HUE, CENTERPULSE_DEFAULT_SAT, CENTERPULSE_DEFAULT_VAL),
            speedDiv(6) {
    name = CENTERPULSE_ANIMATION_NAME;
}

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
        if (d <= radius) {
            vRow = val; // fully lit inside and on current radius
        } else if (d == (radius + 1)) {
            // frontier row beyond current radius: blend in progressively
            vRow = scale8(val, frac);
        } else {
            continue;
        }
        for (int x = 0; x < w; ++x) {
            matrix->setPixelHSV(x, y, hue, sat, vRow);
        }
    }
    matrix->show();
}

bool CenterPulseAnimation::serialize(uint8_t* out, size_t maxLen) const {
    if (!out || maxLen < 2) return false;
    out[0] = hue;
    out[1] = sat;
    return true;
}

bool CenterPulseAnimation::deserialize(const uint8_t* data, size_t len) {
    if (!data || len < 2) return false;
    setColorHSV(data[0], data[1], ANIMATION_DEFAULT_VAL);
    return true;
}
