#ifndef EQUALIZER_BARS_ANIMATION_HPP
#define EQUALIZER_BARS_ANIMATION_HPP
#include <Arduino.h>
#include "../../LedMatrix/LedMatrix.hpp"
#include "../../Animation/Animation.hpp"

#ifndef EQ_DEFAULT_HUE
#define EQ_DEFAULT_HUE 96 // green
#endif
#ifndef EQ_DEFAULT_SAT
#define EQ_DEFAULT_SAT 255
#endif
#ifndef EQ_DEFAULT_VAL
#define EQ_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class EqualizerBarsAnimation : public AnimationBase {
public:
    explicit EqualizerBarsAnimation(LedMatrix& m);
    void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
    void setSpeedDiv(uint8_t div);
    void setJitter(uint8_t j); // randomness of target changes
    void render() override;
private:
    uint8_t speedDiv;
    uint8_t jitter;
    uint8_t heights[MATRIX_WIDTH];      // current height per column (0..H)
    int8_t  velocity[MATRIX_WIDTH];      // per-column velocity for continuous motion
    uint8_t step;                        // base step size
    uint32_t nextStepMs;
    uint16_t stepPeriodMs;
};

#endif // EQUALIZER_BARS_ANIMATION_HPP
