#ifndef SEGMENT_RUNNER_ANIMATION_HPP
#define SEGMENT_RUNNER_ANIMATION_HPP

#include <Arduino.h>
#include "../Animation/Animation.hpp"

#ifndef SEGMENTRUNNER_DEFAULT_HUE
#define SEGMENTRUNNER_DEFAULT_HUE 170 // blue
#endif
#ifndef SEGMENTRUNNER_DEFAULT_SAT
#define SEGMENTRUNNER_DEFAULT_SAT 255
#endif
#ifndef SEGMENTRUNNER_DEFAULT_VAL
#define SEGMENTRUNNER_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class SegmentRunnerAnimation : public AnimationBase {
public:
    explicit SegmentRunnerAnimation(LedMatrix& m);

    void setStepPeriodMs(uint16_t ms);
    void render() override;

private:
    uint16_t stepPeriodMs; // time per row change
};

#endif // SEGMENT_RUNNER_ANIMATION_HPP
