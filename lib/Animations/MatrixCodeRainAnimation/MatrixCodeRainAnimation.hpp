#ifndef MATRIX_CODE_RAIN_ANIMATION_HPP
#define MATRIX_CODE_RAIN_ANIMATION_HPP
#include <Arduino.h>
#include "../../LedMatrix/LedMatrix.hpp"
#include "../../Animation/Animation.hpp"

#ifndef MATRIX_RAIN_DEFAULT_HUE
#define MATRIX_RAIN_DEFAULT_HUE 96 // green-ish
#endif
#ifndef MATRIX_RAIN_DEFAULT_SAT
#define MATRIX_RAIN_DEFAULT_SAT 255
#endif
#ifndef MATRIX_RAIN_DEFAULT_VAL
#define MATRIX_RAIN_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class MatrixCodeRainAnimation : public AnimationBase {
public:
    explicit MatrixCodeRainAnimation(LedMatrix& m);
    void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
    void setSpeedDiv(uint8_t div);
    void setTailLen(uint8_t len);
    void render() override;
private:
    uint8_t speedDiv; // divider for time progression
    uint8_t tailLen;  // trail length in rows
    int heads[MATRIX_WIDTH]; // per-column head position (-tail..h-1)
    uint32_t nextStepMs; // next step timestamp
    uint16_t stepPeriodMs; // step interval in ms
};

#endif // MATRIX_CODE_RAIN_ANIMATION_HPP
