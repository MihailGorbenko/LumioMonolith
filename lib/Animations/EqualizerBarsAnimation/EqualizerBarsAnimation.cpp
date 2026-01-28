#include "EqualizerBarsAnimation.hpp"
#include <FastLED.h>

EqualizerBarsAnimation::EqualizerBarsAnimation(uint16_t id)
    : AnimationBase(EQ_DEFAULT_HUE, id),
        speedDiv(2), step(1), nextStepMs(0), stepPeriodMs(30),
        numCols(0), numRows(0) {
    int w = 0;
    int h = 0;
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

void EqualizerBarsAnimation::render(LedMatrix& m) {
    int w = m.getWidth();
    int h = m.getHeight();
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

    m.clear();
    // draw columns from bottom using mapped column count; solid color (no per-pixel gradient)
    for (int x = 0; x < numCols; ++x) {
        uint8_t colHeight = heights[x];
        for (int y = 0; y < numRows; ++y) {
            bool lit = (y >= (numRows - colHeight)); // bottom-up
            if (lit) {
                if (animCfg.hue > 250) {
                    // synchronized ascending rainbow gradient by row (bottom->top) across all columns
                    int rowsRange = (numRows > 1) ? (numRows - 1) : 1;
                    uint8_t posFromBottom = (uint8_t)(numRows - 1 - y); // 0 at bottom -> rowsRange at top
                    uint8_t hOut = (uint8_t)((posFromBottom * 255) / rowsRange);
                    m.setPixelHSV(x, y, hOut, 255, ANIMATION_DEFAULT_VAL);
                } else {
                    m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, ANIMATION_DEFAULT_VAL);
                }
            }
        }
    }

    // show() is managed by AppController
}

// Base class provides ISerializable
