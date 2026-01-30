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
    void render(LedMatrix& m) override;
      const char* getName() const override { return SEGMENTRUNNER_ANIMATION_NAME; }

private:
    uint16_t stepPeriodMs; // time per row change

};

#endif // SEGMENT_RUNNER_ANIMATION_HPP
