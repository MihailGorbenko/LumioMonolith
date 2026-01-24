#include "MatrixCodeRainAnimation.hpp"
#include <FastLED.h>

MatrixCodeRainAnimation::MatrixCodeRainAnimation(LedMatrix& m)
        : AnimationBase(m, MATRIX_RAIN_DEFAULT_HUE, MATRIX_RAIN_DEFAULT_SAT, MATRIX_RAIN_DEFAULT_VAL),
            speedDiv(3), tailLen(3), nextStepMs(0), stepPeriodMs(50) {
        int w = m.getWidth();
        int h = m.getHeight();
        tailLen = (uint8_t)min<uint8_t>(tailLen, (h > 0 ? (h - 1) : 1));
        for (int x = 0; x < w; ++x) {
                heads[x] = random8(0, (h > 0 ? h : 1)); // start within visible area for constant presence
        }
}

void MatrixCodeRainAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
    AnimationBase::setColorHSV(h, s, v);
}

void MatrixCodeRainAnimation::setSpeedDiv(uint8_t div) {
    speedDiv = (div == 0) ? 1 : div;
}

void MatrixCodeRainAnimation::setTailLen(uint8_t len) {
    tailLen = (len == 0) ? 1 : len;
}

void MatrixCodeRainAnimation::render() {
    if (!matrix) return;
    int w = matrix->width();
    int h = matrix->height();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    uint32_t now = millis();
    if (now >= nextStepMs) {
        nextStepMs = now + (stepPeriodMs * speedDiv);
        // continuous top->down movement, wrap within visible area
        for (int x = 0; x < w; ++x) {
            int head = heads[x] + 1;
            if (head >= h) head = 0;
            heads[x] = head;
        }
    }

    matrix->clear();
    // draw columns with tail falloff (constant per-column presence)
    for (int x = 0; x < w; ++x) {
        int head = heads[x];
        for (int t = 0; t <= tailLen; ++t) {
            int y = head - t;
            if (y < 0 || y >= h) continue;
            // brightness falloff for tail
            uint8_t fall = (uint8_t)(255 - (t * (255 / (tailLen + 1))));
            uint8_t b = (t == 0) ? val : scale8(val, fall);
            // slight hue shift for head vs tail
            uint8_t huseg = (t == 0) ? hue : hue - 8;
            matrix->setPixelHSV(x, y, huseg, sat, b);
        }
    }

    matrix->show();
}
