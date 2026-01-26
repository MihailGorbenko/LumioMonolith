#ifndef EQUALIZER_BARS_ANIMATION_HPP
#define EQUALIZER_BARS_ANIMATION_HPP
#include <Arduino.h>
#include <vector>
#include "../../LedMatrix/LedMatrix.hpp"
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef EQ_DEFAULT_HUE
#define EQ_DEFAULT_HUE 96 // green
#endif
#ifndef EQ_DEFAULT_SAT
#define EQ_DEFAULT_SAT 255
#endif
#ifndef EQ_DEFAULT_VAL
#define EQ_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Readable name
#ifndef EQUALIZERBARS_ANIMATION_NAME
#define EQUALIZERBARS_ANIMATION_NAME "Equalizer Bars"
#endif

class EqualizerBarsAnimation : public AnimationBase, public ISerializable {
public:
    explicit EqualizerBarsAnimation(LedMatrix& m);
    // configuration setters removed as unused
    void render() override;
private:
    uint8_t speedDiv;
    int numCols;
    int numRows;
    std::vector<uint8_t> heights;      // current height per column (0..H)
    std::vector<int8_t> velocity;      // per-column velocity for continuous motion
    uint8_t step;                        // base step size
    uint32_t nextStepMs;
    uint16_t stepPeriodMs;

    // ISerializable
public:
    size_t serializedSize() const override { return 2; }
    bool serialize(uint8_t* out, size_t maxLen) const override;
    bool deserialize(const uint8_t* data, size_t len) override;
    ISerializable* serializable() override { return this; }
};

#endif // EQUALIZER_BARS_ANIMATION_HPP
