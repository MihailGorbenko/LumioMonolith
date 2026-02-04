#include "ReactorTurbinesAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"
#include <math.h>
#include <FastLED.h>

ReactorTurbinesAnimation::ReactorTurbinesAnimation(uint16_t id, LedMatrix& m)
    : AnimationBase(REACTOR_DEFAULT_HUE, id, m) {}

void ReactorTurbinesAnimation::onActivate() {
    LedMatrix& m = matrix;
    int w = m.getWidth();
    int h = m.getHeight();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
    // Clamp to a practical maximum to avoid huge allocations
    const int MAX_DIM = 256;
    if (w > MAX_DIM) w = MAX_DIM;
    if (h > MAX_DIM) h = MAX_DIM;

    cachedWidth = w;
    cachedHeight = h;

    segLen.assign(h, 0);
    phase.assign(h, 0);
    speedFactor.assign(h, 1.0f);

    const float center = (h - 1) * 0.5f;
    const int baseLen = REACTOR_SEGMENT_LEN;
    const int maxDelta = 2;

    for (int y = 0; y < h; ++y) {
        int delta = (int)roundf((h > 1 ? fabsf(y - center) / center : 0.f) * maxDelta);
        segLen[y] = max(1, baseLen - delta);
        phase[y] = (y * cachedWidth) / cachedHeight;
        speedFactor[y] = 1.0f + 0.12f * sinf(6.2831853f * ((y + 0.5f) / (float)cachedHeight));
    }

    vBright = ANIMATION_DEFAULT_VAL;
    vDim = scale8(ANIMATION_DEFAULT_VAL, REACTOR_DIM_SCALE);

    AnimationBase::onActivate();
}

void ReactorTurbinesAnimation::render() {
    if (!isInitialized()) return;
    LedMatrix& m = matrix;

    const int w = cachedWidth;
    const int h = cachedHeight;
    if (w <= 0 || h <= 0) return;

    m.clear();

    const uint32_t now = millis();
    const float speedDiv = (REACTOR_SPEED_MS == 0) ? 1.0f : (float)REACTOR_SPEED_MS;
    const float t = now / speedDiv;

    for (int y = 0; y < h; ++y) {
        int seg = (y < (int)segLen.size()) ? segLen[y] : REACTOR_SEGMENT_LEN;
        int basePhase = (y < (int)phase.size()) ? phase[y] : ((y * w) / h);
        float sf = (y < (int)speedFactor.size()) ? speedFactor[y] : 1.0f;

        // bound the computed step to [0, w) using fmodf to avoid casting
        // a large float (derived from millis()) into a plain `int` which
        // could overflow on some platforms where `int` is 16-bit.
        int32_t stepRow = (int32_t)fmodf(t * sf, (float)w);
        int pos = (basePhase + stepRow) % w;
        pos = (pos + w) % w;

        int posRight = pos;
        int posLeft = w - 1 - pos;

        // Draw dim (left-moving) first so bright overwrites on overlap
        for (int i = 0; i < seg; ++i) {
            int x = posLeft - i;
            while (x < 0) x += w;
            x %= w;
            m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vDim);
        }
        // Draw bright (right-moving) turbine
        for (int i = 0; i < seg; ++i) {
            int x = (posRight + i) % w;
            m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vBright);
        }
    }

}

// Base class provides ISerializable
