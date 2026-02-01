#include "MatrixCodeRainAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>

MatrixCodeRainAnimation::MatrixCodeRainAnimation(uint16_t id)
        : AnimationBase(MATRIX_RAIN_DEFAULT_HUE, id),
            numCols(0), numRows(0), nextStepMs(0), stepPeriodMs(25) {
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
        gaps.assign(numCols, 0);
        for (int x = 0; x < numCols; ++x) {
            heads[x] = random8(0, (uint8_t)(numRows > 0 ? numRows : 1));
            // faster per-column speeds with variety
            // smaller range and allow 1 to make some drops noticeably faster
            speeds[x] = (uint8_t)random8(1, 7); // 1..6 ticks (lower -> faster)
            counter[x] = random8(speeds[x]);
            // mostly 2-3, occasionally 4 (cap at 4)
            uint8_t t = (random8(0, 10) < 8) ? (uint8_t)random8(2, 4) : 4;
            if (t > 4) t = 4;
            // Limit tail length relative to available rows so a tail can't fill the
            // whole column via wrap. Keep at least 1 for very small matrices.
            uint8_t maxTail = (numRows > 2) ? (uint8_t)(numRows - 2) : 1;
            if (t > maxTail) t = maxTail;
            tailLens[x] = t;
            // Start with a per-column random gap so columns are staggered.
            // Use a modest range relative to height so faster drops remain visible.
            gaps[x] = random8(0, (uint8_t)max(6, (int)numRows));
        }
    }

    uint32_t now = millis();
    if ((int32_t)(now - nextStepMs) >= 0) {
        nextStepMs = now + stepPeriodMs;
        // движение вниз по Y: каждая колонка с собственной скоростью
        for (int x = 0; x < numCols; ++x) {
            // enforce a small random gap between successive drops in the same column
            if (gaps[x] > 0) {
                --gaps[x];
                continue;
            }
            counter[x]++;
            if (counter[x] >= speeds[x]) {
                counter[x] = 0;
                int head = heads[x] + 1;
                bool wrapped = false;
                if (head >= numRows) { head = 0; wrapped = true; }
                heads[x] = head;
                // after a move, set a (smaller) random gap before next move so
                // intervals vary but drops remain snappy at higher speed.
                uint8_t g = random8(1, 12); // gap 1..11 ticks
                gaps[x] = g;
                // nudge neighbors to avoid immediate adjacent drops
                uint8_t ng = random8(2, 6); // neighbor gap 2..5
                if (x > 0) gaps[x - 1] = max<uint8_t>(gaps[x - 1], ng);
                if (x + 1 < numCols) gaps[x + 1] = max<uint8_t>(gaps[x + 1], ng);
                // more often adjust speed for dynamic variation and allow very fast drops
                if (random8(0, 100) < 30) { // ~30% chance
                    speeds[x] = (uint8_t)random8(1, 7);
                }
                // always refresh speed when wrapping to top to keep variety
                if (wrapped) {
                    speeds[x] = (uint8_t)random8(1, 7);
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
        for (int t = 0; t <= tlen; ++t) {
            int y = head - t;
            // Stop the tail at the top — do not wrap around to the bottom.
            if (y < 0) break;
            uint8_t vpix = (t == 0) ? ANIMATION_DEFAULT_VAL : scale8(ANIMATION_DEFAULT_VAL, 180);
            m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vpix);
        }
    }

}

// Base class provides ISerializable
