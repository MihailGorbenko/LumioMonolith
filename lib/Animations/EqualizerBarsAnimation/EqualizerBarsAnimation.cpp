#include "EqualizerBarsAnimation.hpp"
#include <FastLED.h>

EqualizerBarsAnimation::EqualizerBarsAnimation(LedMatrix& m)
        : AnimationBase(m, EQ_DEFAULT_HUE, EQ_DEFAULT_SAT, EQ_DEFAULT_VAL),
                speedDiv(2), step(1), nextStepMs(0), stepPeriodMs(30),
            numCols(0), numRows(0) {
    int w = m.getWidth();
    int h = m.getHeight();
            numCols = min(w > 0 ? w : 1, MATRIX_WIDTH);
    numRows = h > 0 ? h : 1;
    heights.resize(numCols);
    velocity.resize(numCols);
    for (int x = 0; x < numCols; ++x) {
        heights[x] = random8(0, numRows + 1);
        velocity[x] = (int8_t)((int8_t)random8(0, 3) - 1); // -1..+1
    }
}

// configuration setters removed as unused

void EqualizerBarsAnimation::render() {
    if (!matrix) return;
    int w = matrix->width();
    int h = matrix->height();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
    // resize vectors if matrix size changed
    if (w != numCols || h != numRows) {
        numCols = min(w, MATRIX_WIDTH);
        numRows = h;
        heights.resize(numCols);
        velocity.resize(numCols);
        for (int x = 0; x < numCols; ++x) {
            heights[x] = random8(0, numRows + 1);
            velocity[x] = (int8_t)((int8_t)random8(0, 3) - 1);
        }
    }

    uint32_t now = millis();
    if ((int32_t)(now - nextStepMs) >= 0) {
        nextStepMs = now + (stepPeriodMs * speedDiv);
        // continuous per-column smooth motion: small random accel, no sudden jumps
        for (int x = 0; x < numCols; ++x) {
            int8_t dv = (int8_t)((int8_t)random8(0, 3) - 1); // -1..+1
            velocity[x] = (int8_t)constrain((int)velocity[x] + dv, -2, 2);
            int nh = (int)heights[x] + (int)velocity[x] * (int)step;
            if (nh < 0) { nh = 0; velocity[x] = (int8_t)abs(velocity[x]); }
            if (nh > numRows) { nh = numRows; velocity[x] = (int8_t)(-abs(velocity[x])); }
            heights[x] = (uint8_t)nh;
        }
    }

    matrix->clear();
    // draw columns from bottom using mapped column count; solid color (no per-pixel gradient)
    for (int x = 0; x < numCols; ++x) {
        uint8_t colHeight = heights[x];
        for (int y = 0; y < numRows; ++y) {
            bool lit = (y >= (numRows - colHeight)); // bottom-up
            if (lit) {
                if (hue > 250) {
                    // synchronized ascending rainbow gradient by row (bottom->top) across all columns
                    int rowsRange = (numRows > 1) ? (numRows - 1) : 1;
                    uint8_t posFromBottom = (uint8_t)(numRows - 1 - y); // 0 at bottom -> rowsRange at top
                    uint8_t hOut = (uint8_t)((posFromBottom * 255) / rowsRange);
                    matrix->setPixelHSV(x, y, hOut, 255, val);
                } else {
                    matrix->setPixelHSV(x, y, hue, sat, val);
                }
            }
        }
    }

    matrix->show();
}
