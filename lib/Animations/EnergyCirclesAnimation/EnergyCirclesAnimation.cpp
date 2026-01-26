#include "EnergyCirclesAnimation.hpp"
#include <math.h>

EnergyCirclesAnimation::EnergyCirclesAnimation(LedMatrix& m)
    : AnimationBase(m, ENERGY_DEFAULT_HUE, ENERGY_DEFAULT_SAT, ENERGY_DEFAULT_VAL) {
    name = ENERGYCIRCLES_ANIMATION_NAME;
}

void EnergyCirclesAnimation::render() {
    if (!matrix) return;

    const int w = matrix->width();
    const int h = matrix->height();
    if (w <= 0 || h <= 0) return;

    matrix->clear();

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
                matrix->setPixelHSV(x, y, hue, sat, val);
            }
        } else {
            startX = w - 1 - pos;
            for (int i = 0; i < segLen; ++i) {
                int x = startX - i;
                while (x < 0) x += w;
                x %= w;
                matrix->setPixelHSV(x, y, hue, sat, val);
            }
        }
    }

    matrix->show();
}

bool EnergyCirclesAnimation::serialize(uint8_t* out, size_t maxLen) const {
    if (!out || maxLen < 2) return false;
    out[0] = hue;
    out[1] = sat;
    return true;
}

bool EnergyCirclesAnimation::deserialize(const uint8_t* data, size_t len) {
    if (!data || len < 2) return false;
    setColorHSV(data[0], data[1], ANIMATION_DEFAULT_VAL);
    return true;
}
