#include "EnergyCirclesAnimation.hpp"
#include <math.h>
#include "../../LedMatrix/LedMatrix.hpp"

EnergyCirclesAnimation::EnergyCirclesAnimation(uint16_t id, LedMatrix& m)
    : AnimationBase(ENERGY_DEFAULT_HUE, id, m) {}
void EnergyCirclesAnimation::onActivate() {
    LedMatrix& m = matrix;
    int w = m.getWidth();
    int h = m.getHeight();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
    const int MAX_DIM = 256;
    if (w > MAX_DIM) w = MAX_DIM;
    if (h > MAX_DIM) h = MAX_DIM;

    cachedWidth = w;
    cachedHeight = h;

    segLen.assign(h, 0);
    phase.assign(h, 0);
    speedFactor.assign(h, 1.0f);
    dirRight.assign(h, 1);

    for (int y = 0; y < h; ++y) {
        dirRight[y] = (uint8_t)((y % 2) == 0 ? 1 : 0);
        const int baseLen = ENERGY_SEGMENT_LEN;
        const float center = (h - 1) * 0.5f;
        const int maxDelta = 2;
        int delta = (int)roundf((h > 1 ? fabsf(y - center) / center : 0.f) * maxDelta);
        segLen[y] = max(1, baseLen - delta);
        phase[y] = (y * w) / h;
        speedFactor[y] = 1.0f + 0.15f * sinf(6.2831853f * ((y + 0.5f) / (float)h));
    }

    AnimationBase::onActivate();
}

void EnergyCirclesAnimation::render() {
    if (!isInitialized()) return;
    LedMatrix& m = matrix;

    const int w = cachedWidth;
    const int h = cachedHeight;

    m.clear();

    const uint32_t now = millis();
    const float speedMs = (ENERGY_SPEED_MS == 0) ? 1.0f : (float)ENERGY_SPEED_MS;
    const float t = now / speedMs;

    for (int y = 0; y < h; ++y) {
        const bool right = (dirRight[y] != 0);
        const int rowSegLen = segLen[y];
        const int rowPhase = phase[y];
        const float rowSpeed = speedFactor[y];

        // bound the computed step to [0, w) using fmodf to avoid casting
        // a large float (derived from millis()) into a plain `int` which
        // could overflow on some platforms where `int` is 16-bit.
        int32_t stepRow = (int32_t)fmodf(t * rowSpeed, (float)w);
        int pos = (rowPhase + stepRow) % w;
        pos = (pos + w) % w;

        if (right) {
            int startX = pos;
            for (int i = 0; i < rowSegLen; ++i) {
                int x = (startX + i) % w;
                m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, ANIMATION_DEFAULT_VAL);
            }
        } else {
            int startX = w - 1 - pos;
            for (int i = 0; i < rowSegLen; ++i) {
                int x = startX - i;
                while (x < 0) x += w;
                x %= w;
                m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, ANIMATION_DEFAULT_VAL);
            }
        }
    }

}

// Base class provides ISerializable
