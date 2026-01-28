#ifndef SEGMENT_RUNNER_ANIMATION_HPP
#define SEGMENT_RUNNER_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef SEGMENTRUNNER_DEFAULT_HUE
#define SEGMENTRUNNER_DEFAULT_HUE 170 // blue
#endif
#ifndef SEGMENTRUNNER_DEFAULT_SAT
#define SEGMENTRUNNER_DEFAULT_SAT 255
#endif
#ifndef SEGMENTRUNNER_DEFAULT_VAL
#define SEGMENTRUNNER_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Readable name
#ifndef SEGMENTRUNNER_ANIMATION_NAME
#define SEGMENTRUNNER_ANIMATION_NAME "Segment Runner"
#endif

class SegmentRunnerAnimation : public AnimationBase {
public:
    explicit SegmentRunnerAnimation(uint16_t id);

    void setStepPeriodMs(uint16_t ms);
    void render(LedMatrix& m) override;

private:
    uint16_t stepPeriodMs; // time per row change

    // ISerializable
public:
    const char* getName() const override { return SEGMENTRUNNER_ANIMATION_NAME; }
    const char* getNvsKeyName() const override { return "segrunner"; }
};

#endif // SEGMENT_RUNNER_ANIMATION_HPP
