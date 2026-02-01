#include "SegmentRunnerAnimation.hpp"
#include "../../LedMatrix/LedMatrix.hpp"

SegmentRunnerAnimation::SegmentRunnerAnimation(uint16_t id)
    : AnimationBase(SEGMENTRUNNER_DEFAULT_HUE, id),
            stepPeriodMs(130) {}


void SegmentRunnerAnimation::render(LedMatrix& m) {
    int w = m.getWidth();
    int h = m.getHeight();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    // Ping-pong head across rows: top -> bottom -> top
    uint32_t step = millis() / (uint32_t)stepPeriodMs;
    uint32_t span = (uint32_t)((h > 1) ? (h - 1) : 0);
    uint32_t phase = (span > 0) ? (step % (2 * span)) : 0;
    int head = (span == 0) ? 0 : ((phase <= span) ? (int)phase : (int)(2 * span - phase));

    m.clear();
    for (int x = 0; x < w; ++x) {
        m.setPixelHSV(x, head, animCfg.hue, ANIMATION_DEFAULT_SAT, ANIMATION_DEFAULT_VAL);
    }

}

// Base class provides ISerializable
