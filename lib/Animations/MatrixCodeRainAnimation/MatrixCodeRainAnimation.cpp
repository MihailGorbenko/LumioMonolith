#include "MatrixCodeRainAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>

MatrixCodeRainAnimation::MatrixCodeRainAnimation(uint16_t id)
        : AnimationBase(MATRIX_RAIN_DEFAULT_HUE, id),
                    numCols(0), numRows(0), nextStepMs(0), stepPeriodMs(40) {
    // Defer initialization until render when matrix size is known.
}

void MatrixCodeRainAnimation::render(LedMatrix& m) {
    int w = m.getWidth();
    int h = m.getHeight();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    // Lazy init on first render or when size changes
    if (numCols != w || numRows != h || heads.size() != (size_t)w) {
        numCols = w;
        numRows = h;
        heads.assign(numCols, 0);
        counter.assign(numCols, 0);
        speeds.assign(numCols, 0);
        // Initialize per-column state and per-column tail lengths
        tailLens.assign(numCols, 2);
        for (int x = 0; x < numCols; ++x) {
            heads[x] = random8(0, (uint8_t)(numRows > 0 ? numRows : 1));
            // faster per-column speeds with variety
            // smaller range and allow 1 to make some drops noticeably faster
            speeds[x] = (uint8_t)random8(1, 7); // 1..6 ticks (lower -> faster)
            counter[x] = random8(speeds[x]);
            // tail length 2..3 (random8 upper bound exclusive)
            uint8_t t = (uint8_t)random8(2, 4);
            // Limit tail length relative to available rows so a tail can't fill the
            // whole column via wrap. Keep at least 1 for very small matrices.
            uint8_t maxTail = (numRows > 2) ? (uint8_t)(numRows - 2) : 1;
            if (t > maxTail) t = maxTail;
            tailLens[x] = t;
            // per-column speeds: 3..6 ticks (lower -> faster). Slightly slower overall.
            speeds[x] = (uint8_t)random8(3, 7); // 3..6
            // start counter offset so columns are staggered
            counter[x] = random8(speeds[x]);
        }
    }

    uint32_t now = millis();
    if ((int32_t)(now - nextStepMs) >= 0) {
        nextStepMs = now + stepPeriodMs;
        // движение вниз по Y: каждая колонка с собственной скоростью
        for (int x = 0; x < numCols; ++x) {
            // very rare micro acceleration to add subtle life (~1%)
            if (random8(0, 255) < 3) {
                counter[x]++;
            }
            counter[x]++;
            if (counter[x] >= speeds[x]) {
                counter[x] = 0;
                int head = heads[x] + 1;
                bool wrapped = false;
                if (head >= numRows) { head = 0; wrapped = true; }
                heads[x] = head;
                // Change speed only when wrapping to top to avoid mid-fall jumps
                if (wrapped) {
                    speeds[x] = (uint8_t)random8(3, 7); // 3..6
                }
            }
        }
    }

    m.clear();
    // вертикальный дождь сверху вниз: капли через столбец (чистый цвет)
    for (int x = 0; x < numCols; ++x) {
        // render only every second column visually to keep the original sparse look
        if ((x & 1) != 0) continue;
        int head = heads[x];
        // Don't allow tail to be longer than the physical column minus 2
        // so it cannot wrap and fill the column in one frame.
        uint8_t maxTail = (numRows > 2) ? (uint8_t)(numRows - 2) : 1;
        uint8_t tlen = (x < (int)tailLens.size()) ? ((tailLens[x] < maxTail) ? tailLens[x] : maxTail) : 2;
        // Make visual tail one pixel shorter than configured
        if (tlen > 0) tlen = (uint8_t)(tlen - 1);
        for (int t = 0; t <= tlen; ++t) {
            int y = head - t;
            // Stop the tail at the top — do not wrap around to the bottom.
            if (y < 0) break;
            uint8_t vpix;
            if (t == 0) {
                vpix = ANIMATION_DEFAULT_VAL; // head full brightness
            } else {
                // fade tail: start ~180 for first tail pixel, decrease towards min
                const uint8_t maxScale = 180; // scale for t==1
                const uint8_t minScale = 40;  // minimum visible scale at tail end
                uint8_t denom = (tlen > 0) ? tlen : 1;
                // t-1 ranges 0..(tlen-1); compute linear interpolation
                uint8_t range = (uint8_t)(maxScale - minScale);
                uint8_t dec = (uint8_t)(((uint16_t)range * (uint8_t)(t - 1)) / denom);
                uint8_t scale = (dec >= range) ? minScale : (uint8_t)(maxScale - dec);
                if (scale < minScale) scale = minScale;
                vpix = scale8(ANIMATION_DEFAULT_VAL, scale);
            }
            m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vpix);
        }
    }

}

// Base class provides ISerializable
