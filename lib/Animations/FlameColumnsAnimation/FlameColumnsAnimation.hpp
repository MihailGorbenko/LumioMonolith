#ifndef FLAME_COLUMNS_ANIMATION_HPP
#define FLAME_COLUMNS_ANIMATION_HPP
#include <Arduino.h>
#include "../../LedMatrix/LedMatrix.hpp"
#include "../../Animation/Animation.hpp"

#ifndef FLAME_DEFAULT_HUE
#define FLAME_DEFAULT_HUE 16 // orange
#endif
#ifndef FLAME_DEFAULT_SAT
#define FLAME_DEFAULT_SAT 255
#endif
#ifndef FLAME_DEFAULT_VAL
#define FLAME_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class FlameColumnsAnimation : public AnimationBase {
public:
    explicit FlameColumnsAnimation(LedMatrix& m);
    void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
    void setCooling(uint8_t c); // cooling factor
    void setSparking(uint8_t s); // chance of bottom spark
    void setSpeedDiv(uint8_t div);
    void render() override;
private:
    uint8_t cooling;   // 0..255 cooling per step
    uint8_t sparking;  // 0..255 chance of spark
    uint8_t speedDiv;  // speed divisor
    uint32_t nextStepMs;
    uint16_t stepPeriodMs;
    uint8_t heat[MATRIX_WIDTH][MATRIX_HEIGHT];
    void heatStep(int w, int h);
    void drawFlame(int w, int h);
};

#endif // FLAME_COLUMNS_ANIMATION_HPP
