#include "EqualizerBarsAnimation.hpp"
#include <FastLED.h>

EqualizerBarsAnimation::EqualizerBarsAnimation(uint16_t id)
    : AnimationBase(EQ_DEFAULT_HUE, id),
        speedDiv(1), step(1), nextStepMs(0), stepPeriodMs(80),
        numCols(0), numRows(0) {
    int w = 0;
    int h = 0;
    numCols = min(w > 0 ? w : 1, MATRIX_WIDTH);
    numRows = h > 0 ? h : 1;
        heights.resize(numCols);
        targets.resize(numCols);
        groupTargets = {0,0,0};
        for (int x = 0; x < numCols; ++x) {
            heights[x] = (uint8_t)random8(1, numRows + 1);
            targets[x] = heights[x];
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
        targets.resize(numCols);
        for (int x = 0; x < numCols; ++x) {
            heights[x] = (uint8_t)random8(1, numRows + 1);
            targets[x] = heights[x];
        }
    }

    uint32_t now = millis();
    if ((int32_t)(now - nextStepMs) >= 0) {
        nextStepMs = now + (stepPeriodMs * speedDiv);
        // Hard digital equalizer model: targets set occasionally; levels move sharply towards target
        // Update group-level energy for low/mid/high bands
        // Scales: low = 100%, mid = 80%, high = 60% of random strength
        for (int g = 0; g < 3; ++g) {
            uint8_t chance = (g == 0) ? 75 : (g == 1) ? 65 : 45; // slightly reduced activity for medium speed
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
            int g = (x * 3) / max(1, numCols); // 0..2
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
                    int rowsRange = (numRows > 1) ? (numRows - 1) : 1;
                    uint8_t posFromBottom = (uint8_t)(numRows - 1 - y); // 0 at bottom -> rowsRange at top
                    uint8_t hOut = (uint8_t)((posFromBottom * 255) / rowsRange);
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

// Base class provides ISerializable
