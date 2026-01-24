#include "PowerOnAnimation.hpp"

PowerOnAnimation::PowerOnAnimation(LedMatrix& m)
    : matrix(&m),
      hue(POWERON_DEFAULT_HUE),
      sat(POWERON_DEFAULT_SAT),
      val(POWERON_DEFAULT_VAL),
      progress(0) {}

void PowerOnAnimation::setProgress(uint8_t p) { progress = p; }

void PowerOnAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
    hue = h; sat = s; val = v;
}

void PowerOnAnimation::render() {
    if (!matrix) return;
    matrix->clear();

    int w = matrix->width();
    int h = matrix->height();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    // Shot head moves bottom -> top with trailing glow
    // Map progress 0..255 → headY from (h-1)→0 using fixed-point
    uint32_t yf = (uint32_t)(h - 1) * (uint32_t)(255 - progress);
    int headY = (int)(yf / 255);            // 0..h-1
    int rem = (int)(yf % 255);              // 0..254 fractional for blend

    // Center brightness profile for plasma beam (brighter center columns)
    int mid = w / 2;
    for (int y = 0; y < h; ++y) {
        // distance from head for vertical falloff
        int d = abs(y - headY);
        uint8_t vRow;
        if (y == headY) {
            vRow = val; // head max
            // blend next row above for continuity
            if (rem > 0 && headY > 0 && y - 1 >= 0) {
                // handled when iterating that y
            }
        } else if (d == 1) {
            // immediate trail
            vRow = scale8(val, 180);
            // fractional blend: stronger when close to head transition
            if (rem > 0) vRow = qadd8(vRow, (uint8_t)(rem / 3));
        } else if (d == 2) {
            vRow = scale8(val, 90);
        } else {
            vRow = 0;
        }

        for (int x = 0; x < w; ++x) {
            // radial-like horizontal profile: center brighter
            int dx = abs(x - mid);
            uint8_t centerGain = (uint8_t)constrain(255 - dx * 22, 160, 255);
            uint8_t vOut = scale8(vRow, centerGain);

            // slight deterministic flicker for plasma feel
            uint8_t flick = (uint8_t)(((x * 37 + y * 53 + (millis() >> 2)) & 0x0F) * 6);
            vOut = qadd8(vOut, flick);

            if (vOut > 0) {
                matrix->setPixelHSV(x, y, hue, sat, vOut);
            }
        }
    }

    matrix->show();
}
