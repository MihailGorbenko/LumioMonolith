#include "ReactorTurbinesAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <math.h>
#include <FastLED.h>

ReactorTurbinesAnimation::ReactorTurbinesAnimation(uint16_t id, LedMatrix* m)
    : AnimationBase(REACTOR_DEFAULT_HUE, id, m) {}

void ReactorTurbinesAnimation::render() {
    LedMatrix* m = matrix;
    if (!m) return;

    const int w = m->getWidth();
    const int h = m->getHeight();
    if (w <= 0 || h <= 0) return;

    m->clear();

    const uint32_t now = millis();
    const float t = now / (float)REACTOR_SPEED_MS;

    // Center row to vary segment length slightly like concentric rings
    const float center = (h - 1) * 0.5f;
    const int baseLen = REACTOR_SEGMENT_LEN;
    const int maxDelta = 2; // shorter at edges, longest near center

    for (int y = 0; y < h; ++y) {
        // Per-ring segment length variation
        int delta = (int)roundf((h > 1 ? fabsf(y - center) / center : 0.f) * maxDelta);
        int segLen = max(1, baseLen - delta);

        // Unique phase per ring
        int phase = (y * w) / h;
        float speedFactor = 1.0f + 0.12f * sinf(6.2831853f * ((y + 0.5f) / (float)h));
        int stepRow = ((int)(t * speedFactor)) % w;
        int pos = (phase + stepRow) % w;
        pos = (pos + w) % w;

        // Two counter-rotating turbine segments on the same ring
        int posRight = pos;             // moves to the right
        int posLeft  = w - 1 - pos;     // moves to the left

        // Brightness contrast for turbine blades
        uint8_t vBright = ANIMATION_DEFAULT_VAL;                  // leading turbine
        uint8_t vDim    = scale8(ANIMATION_DEFAULT_VAL, REACTOR_DIM_SCALE); // trailing turbine (dimmer)

        // Draw dim (left-moving) first so bright overwrites on overlap
        for (int i = 0; i < segLen; ++i) {
            int x = posLeft - i;
            while (x < 0) x += w;
            x %= w;
            m->setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vDim);
        }
        // Draw bright (right-moving) turbine
        for (int i = 0; i < segLen; ++i) {
            int x = (posRight + i) % w;
            m->setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vBright);
        }
    }

}

// Base class provides ISerializable
