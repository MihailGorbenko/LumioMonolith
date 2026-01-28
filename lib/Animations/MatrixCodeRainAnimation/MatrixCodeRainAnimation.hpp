#ifndef MATRIX_CODE_RAIN_ANIMATION_HPP
#define MATRIX_CODE_RAIN_ANIMATION_HPP
#include <Arduino.h>
#include <vector>
#include "../../LedMatrix/LedMatrix.hpp"
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef MATRIX_RAIN_DEFAULT_HUE
#define MATRIX_RAIN_DEFAULT_HUE 96 // green-ish
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
    ~MatrixCodeRainAnimation() override;
    void setTailLen(uint8_t len);
    void render(LedMatrix& m) override;
    const char* getName() const override { return MATRIXCODERAIN_ANIMATION_NAME; }
    const char* getNvsKeyName() const override { return "matrixrain"; }
private:
    uint8_t tailLen;  // trail length in rows
    std::vector<int> heads; // per-column head position (y)
    std::vector<uint8_t> counter; // per-column counter for independent timing
    std::vector<uint8_t> speeds; // per-column step speed (ticks between moves)
    int numCols; // number of columns (width)
    int numRows; // number of rows (height)
    uint32_t nextStepMs; // next step timestamp
    uint16_t stepPeriodMs; // step interval in ms
};

#endif // MATRIX_CODE_RAIN_ANIMATION_HPP
