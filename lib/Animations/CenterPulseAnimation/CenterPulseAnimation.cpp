#include "CenterPulseAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>

CenterPulseAnimation::CenterPulseAnimation(uint16_t id, LedMatrix& m)
    : AnimationBase(CENTERPULSE_DEFAULT_HUE, id, m),
            speedDiv(6) {}


void CenterPulseAnimation::onActivate() {
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
    centerIndex = (h - 1) / 2;
    maxR = max(centerIndex, (h - 1) - centerIndex);
    if (maxR < 0) maxR = 0;

    rowDist.assign(h, 0);
    for (int y = 0; y < h; ++y) rowDist[y] = abs(y - centerIndex);

    AnimationBase::onActivate();
}

void CenterPulseAnimation::render() {
    if (!isInitialized()) return;
    LedMatrix& m = matrix;

    const int w = cachedWidth;
    const int h = cachedHeight;

    // Compute expanding radius 0..maxR using sin wave
    uint8_t phase = (uint8_t)(millis() / speedDiv);
    uint8_t s = sin8(phase); // 0..255
    uint16_t sr = (uint16_t)s * (uint16_t)maxR; // 0..maxR*255
    int radius = (int)(sr / 255);
    uint8_t frac = (uint8_t)(sr % 255); // fractional part for edge blend

    m.clear();
    for (int y = 0; y < h; ++y) {
        int d = rowDist[y];
        uint8_t vRow = 0;
        if (d <= radius) {
            vRow = ANIMATION_DEFAULT_VAL;
        } else if (d == (radius + 1)) {
            vRow = scale8(ANIMATION_DEFAULT_VAL, frac);
        } else {
            continue;
        }
        for (int x = 0; x < w; ++x) {
            m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vRow);
        }
    }

}
