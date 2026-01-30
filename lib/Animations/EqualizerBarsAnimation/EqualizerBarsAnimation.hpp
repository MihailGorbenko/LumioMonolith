#ifndef EQUALIZER_BARS_ANIMATION_HPP
#define EQUALIZER_BARS_ANIMATION_HPP
#include <Arduino.h>
#include <vector>
#include "../../LedMatrix/LedMatrix.hpp"
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef EQ_DEFAULT_HUE
#define EQ_DEFAULT_HUE 85 // classic green (~120° on 0–255 hue scale)
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

class EqualizerBarsAnimation : public AnimationBase {
public:
    explicit EqualizerBarsAnimation(uint16_t id);
    // configuration setters removed as unused
    void render(LedMatrix& m) override;
     const char* getName() const override { return EQUALIZERBARS_ANIMATION_NAME; }
private:
    uint8_t speedDiv;
    int numCols;
    int numRows;
    std::vector<uint8_t> heights;      // current height per column (0..H)
    std::vector<int8_t> velocity;      // per-column velocity for continuous motion
    uint8_t step;                        // base step size
    uint32_t nextStepMs;
    uint16_t stepPeriodMs;

   
};

#endif // EQUALIZER_BARS_ANIMATION_HPP
