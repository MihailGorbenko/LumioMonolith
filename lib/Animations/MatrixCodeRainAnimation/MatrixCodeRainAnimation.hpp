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
    ~MatrixCodeRainAnimation() override;
    void setTailLen(uint8_t len);
    void render() override;
private:
    uint8_t tailLen;  // trail length in rows
    int* heads; // per-column head position (y)
    uint8_t* counter; // per-column counter for independent timing
    uint8_t* speeds; // per-column step speed (ticks between moves)
    int numCols; // number of columns (width)
    int numRows; // number of rows (height)
    uint32_t nextStepMs; // next step timestamp
    uint16_t stepPeriodMs; // step interval in ms
};

#endif // MATRIX_CODE_RAIN_ANIMATION_HPP
