#ifndef SEGMENT_RUNNER_ANIMATION_HPP
#define SEGMENT_RUNNER_ANIMATION_HPP

#include <Arduino.h>
#include "../Animation/Animation.hpp"
#include "../StorageManager/Serializable.hpp"

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

class SegmentRunnerAnimation : public AnimationBase, public ISerializable {
public:
    explicit SegmentRunnerAnimation(LedMatrix& m);

    void setStepPeriodMs(uint16_t ms);
    void render() override;

private:
    uint16_t stepPeriodMs; // time per row change

    // ISerializable
public:
    size_t serializedSize() const override { return 2; }
    bool serialize(uint8_t* out, size_t maxLen) const override;
    bool deserialize(const uint8_t* data, size_t len) override;
    ISerializable* serializable() override { return this; }
};

#endif // SEGMENT_RUNNER_ANIMATION_HPP
