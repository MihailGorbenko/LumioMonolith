#include "MatrixCodeRainAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <FastLED.h>

MatrixCodeRainAnimation::MatrixCodeRainAnimation(uint16_t id)
    : AnimationBase(MATRIX_RAIN_DEFAULT_HUE, id),
      tailLen(1), numCols(0), numRows(0), nextStepMs(0), stepPeriodMs(50) {
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
        // Tail length limited by rows; choose a small default and clamp
        uint8_t maxTail = (numRows > 0) ? (uint8_t)(numRows - 1) : 1;
        uint8_t defTail = 3;
        tailLen = (uint8_t)min<uint8_t>(defTail, maxTail);
        // Initialize per-column state
        for (int x = 0; x < numCols; ++x) {
            heads[x] = random8(0, (uint8_t)(numRows > 0 ? numRows : 1));
            speeds[x] = (uint8_t)random8(4, 13);
            counter[x] = random8(speeds[x]);
        }
    }

    uint32_t now = millis();
    if ((int32_t)(now - nextStepMs) >= 0) {
        nextStepMs = now + stepPeriodMs;
        // движение вниз по Y с индивидуальным таймингом для каждой второй колонки (через столбец)
        for (int x = 0; x < numCols; x += 2) {
            counter[x]++;
            if (counter[x] >= speeds[x]) {
                counter[x] = 0;
                int head = heads[x] + 1;
                if (head >= numRows) head = 0;
                heads[x] = head;
            }
        }
    }

    m.clear();
    // вертикальный дождь сверху вниз: капли через столбец (чистый цвет)
    for (int x = 0; x < numCols; x += 2) {
        int head = heads[x];
        for (int t = 0; t <= tailLen; ++t) {
            int y = head - t;
            if (y < 0) y += numRows; // wrap around
            if (y >= numRows) continue;
            uint8_t vpix = (t == 0) ? ANIMATION_DEFAULT_VAL : scale8(ANIMATION_DEFAULT_VAL, 180); // slight brightness gap for tail
            m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vpix);
        }
    }

}

// Base class provides ISerializable
