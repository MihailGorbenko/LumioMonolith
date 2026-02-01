#ifndef MATRIX_CODE_RAIN_ANIMATION_HPP
#define MATRIX_CODE_RAIN_ANIMATION_HPP
#include <Arduino.h>
#include <vector>
#include "../../LedMatrix/LedMatrix.hpp"
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef MATRIX_RAIN_DEFAULT_HUE
#define MATRIX_RAIN_DEFAULT_HUE 85 // green-ish
#endif
#ifndef MATRIX_RAIN_DEFAULT_SAT
#define MATRIX_RAIN_DEFAULT_SAT 255
#endif
#ifndef MATRIX_RAIN_DEFAULT_VAL
#define MATRIX_RAIN_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Readable name
#ifndef MATRIXCODERAIN_ANIMATION_NAME
#define MATRIXCODERAIN_ANIMATION_NAME "Matrix Code Rain"
#endif

class MatrixCodeRainAnimation : public AnimationBase {
public:
    explicit MatrixCodeRainAnimation(uint16_t id);
    void render(LedMatrix& m) override;
    const char* getName() const override { return MATRIXCODERAIN_ANIMATION_NAME; }
private:
    std::vector<uint8_t> tailLens; // per-column trail length
    std::vector<int> heads; // per-column head position (y)
    std::vector<uint8_t> counter; // per-column counter for independent timing
    std::vector<uint8_t> speeds; // per-column step speed (ticks between moves)
    int numCols; // number of columns (width)
    int numRows; // number of rows (height)
    uint32_t nextStepMs; // next step timestamp
    uint16_t stepPeriodMs; // step interval in ms
};

#endif // MATRIX_CODE_RAIN_ANIMATION_HPP
