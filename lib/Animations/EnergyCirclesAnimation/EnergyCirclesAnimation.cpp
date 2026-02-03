#include "EnergyCirclesAnimation.hpp"
#include <math.h>
#include "../../LedMatrix/LedMatrix.hpp"

EnergyCirclesAnimation::EnergyCirclesAnimation(uint16_t id, LedMatrix& m)
    : AnimationBase(ENERGY_DEFAULT_HUE, id, m) {}

void EnergyCirclesAnimation::render() {
    LedMatrix& m = matrix;

    const int w = m.getWidth();
    const int h = m.getHeight();
    if (w <= 0 || h <= 0) return;

    m->clear();

    // Compute time in "step units" as float for per-row speed
    const uint32_t now = millis();
    const float t = now / (float)ENERGY_SPEED_MS;

    for (int y = 0; y < h; ++y) {
        const bool right = (y % 2 == 0); // even rows move right, odd left
        int startX;

        // Per-row parallax: vary segment length (shorter at edges, longest at center)
        const int baseLen = ENERGY_SEGMENT_LEN;
        const float center = (h - 1) * 0.5f;
        const int maxDelta = 2; // with h=5 and baseLen=10 -> 8/9/10/9/8
        int delta = (int)roundf((h > 1 ? fabsf(y - center) / center : 0.f) * maxDelta);
        const int segLen = max(1, baseLen - delta);

        // Unique phase offset per row
        const int phase = (y * w) / h;

        // Per-row speed variation (subtle)
        const float speedFactor = 1.0f + 0.15f * sinf(6.2831853f * ((y + 0.5f) / (float)h));
        int stepRow = ((int)(t * speedFactor)) % w;

        // Combine phase and per-row speed
        int pos = (phase + stepRow) % w;
        pos = (pos + w) % w;

        if (right) {
            startX = pos;
            for (int i = 0; i < segLen; ++i) {
                int x = (startX + i) % w;
                m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, ANIMATION_DEFAULT_VAL);
            }
        } else {
            startX = w - 1 - pos;
            for (int i = 0; i < segLen; ++i) {
                int x = startX - i;
                while (x < 0) x += w;
                x %= w;
                m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, ANIMATION_DEFAULT_VAL);
            }
        }
    }

}

// Base class provides ISerializable
