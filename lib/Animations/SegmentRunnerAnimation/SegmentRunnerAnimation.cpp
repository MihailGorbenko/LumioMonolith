#include "SegmentRunnerAnimation.hpp"

SegmentRunnerAnimation::SegmentRunnerAnimation(LedMatrix& m)
    : AnimationBase(m, SEGMENTRUNNER_DEFAULT_HUE, SEGMENTRUNNER_DEFAULT_SAT, SEGMENTRUNNER_DEFAULT_VAL),
            stepPeriodMs(130) {
        name = SEGMENTRUNNER_ANIMATION_NAME;
}

void SegmentRunnerAnimation::setStepPeriodMs(uint16_t ms) {
    if (ms == 0) ms = 1;
    stepPeriodMs = ms;
}

void SegmentRunnerAnimation::render() {
    if (!matrix) return;
    int w = matrix->width();
    int h = matrix->height();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    // Ping-pong head across rows: top -> bottom -> top
    uint32_t step = millis() / (uint32_t)stepPeriodMs;
    uint32_t span = (uint32_t)((h > 1) ? (h - 1) : 0);
    uint32_t phase = (span > 0) ? (step % (2 * span)) : 0;
    int head = (span == 0) ? 0 : ((phase <= span) ? (int)phase : (int)(2 * span - phase));

    matrix->clear();
    for (int x = 0; x < w; ++x) {
        matrix->setPixelHSV(x, head, hue, sat, val);
    }
    matrix->show();
}

bool SegmentRunnerAnimation::serialize(uint8_t* out, size_t maxLen) const {
    if (!out || maxLen < 2) return false;
    out[0] = hue;
    out[1] = sat;
    return true;
}

bool SegmentRunnerAnimation::deserialize(const uint8_t* data, size_t len) {
    if (!data || len < 2) return false;
    setColorHSV(data[0], data[1], ANIMATION_DEFAULT_VAL);
    return true;
}
