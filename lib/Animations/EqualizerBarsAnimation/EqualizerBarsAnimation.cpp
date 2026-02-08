#include "EqualizerBarsAnimation.hpp"
#include <FastLED.h>

EqualizerBarsAnimation::EqualizerBarsAnimation(uint16_t id, LedMatrix& m)
    : AnimationBase(EQ_DEFAULT_HUE, id, m),
        speedDiv(1), step(1), nextStepMs(0), stepPeriodMs(80),
        numCols(0), numRows(0), cachedWidth(0), cachedHeight(0), rowsRange(1) {
    // Defer initialization to onActivate() when matrix dimensions are known.
    groupTargets = {0,0,0};
}

// configuration setters removed as unused

void EqualizerBarsAnimation::render() {
    if (!isInitialized()) return;
    LedMatrix& m = matrix;

    // Use cached dimensions populated in onActivate(); do not fallback to
    // querying matrix at render time. If sizes are invalid, bail out.
    const int w = cachedWidth;
    const int h = cachedHeight;

    uint32_t now = millis();
    if ((int32_t)(now - nextStepMs) >= 0) {
        nextStepMs = now + (stepPeriodMs * speedDiv);
        // Update group-level energy for low/mid/high bands
        for (int g = 0; g < 3; ++g) {
            uint8_t chance = (g == 0) ? 75 : (g == 1) ? 65 : 45;
            if (random8() < chance) {
                int base = random8(0, numRows + 1);
                int scale = (g == 0) ? 100 : (g == 1) ? 80 : 60;
                int val = (base * scale) / 100;
                if (val < 1) val = 1;
                groupTargets[g] = (uint8_t)min(numRows, val);
            }
        }
        // Map group targets to per-column targets with small jitter
        for (int x = 0; x < numCols; ++x) {
            int g = colGroup[x]; // precomputed
            int8_t jitter = (int8_t)random8(0,3) - 1; // -1..+1
            int v = (int)groupTargets[g] + (int)jitter;
            if (v < 1) v = 1;
            if (v > numRows) v = numRows;
            targets[x] = (uint8_t)v;

            int diff = (int)targets[x] - (int)heights[x];
            if (diff > 0) {
                heights[x] = (uint8_t)min((int)numRows, (int)heights[x] + min(diff, 1));
            } else if (diff < 0) {
                heights[x] = (uint8_t)max(1, (int)heights[x] - min(-diff, 1));
            }
            if (heights[x] < 1) heights[x] = 1;
        }
    }

    // Hard clear each frame to avoid any persistence/blur — digital equalizer.
    m.clear();
    // draw columns from bottom using mapped column count; solid color (no per-pixel gradient)
    for (int x = 0; x < numCols; ++x) {
        uint8_t colHeight = heights[x];
        for (int y = 0; y < numRows; ++y) {
            bool lit = (y >= (numRows - colHeight)); // bottom-up
            if (lit) {
                if (animCfg.hue > 250) {
                    uint8_t hOut = hueMap[y];
                    m.setPixelHSV(x, y, hOut, 255, ANIMATION_DEFAULT_VAL);
                } else {
                    m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, ANIMATION_DEFAULT_VAL);
                }
            }
        }
        // draw sticky peak (one pixel) above the column (if any)
            // no peaks or persistence — instantaneous digital column
    }

}

void EqualizerBarsAnimation::onActivate() {
    LedMatrix& m = matrix;
    int w = m.getWidth();
    int h = m.getHeight();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
    const int MAX_DIM = 256;
    if (w > MAX_DIM) w = MAX_DIM;
    if (h > MAX_DIM) h = MAX_DIM;

    numCols = min(w, MATRIX_WIDTH);
    numRows = h;
    cachedWidth = numCols;
    cachedHeight = numRows;
    rowsRange = (numRows > 1) ? (numRows - 1) : 1;

    heights.assign(numCols, 0);
    targets.assign(numCols, 0);
    colGroup.assign(numCols, 0);
    hueMap.assign(numRows, 0);

    for (int x = 0; x < numCols; ++x) {
        heights[x] = (uint8_t)random8(1, numRows + 1);
        targets[x] = heights[x];
        colGroup[x] = (uint8_t)((x * 3) / max(1, numCols));
    }

    for (int y = 0; y < numRows; ++y) {
        uint8_t posFromBottom = (uint8_t)(numRows - 1 - y);
        hueMap[y] = (uint8_t)((posFromBottom * 255) / rowsRange);
    }

    groupTargets = {0,0,0};
    nextStepMs = millis() + stepPeriodMs;

    AnimationBase::onActivate();
}

// Base class provides ISerializable
