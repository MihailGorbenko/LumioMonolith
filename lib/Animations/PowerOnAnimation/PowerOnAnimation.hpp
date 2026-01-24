#ifndef POWERON_ANIMATION_HPP
#define POWERON_ANIMATION_HPP

#include <Arduino.h>
#include "../LedMatrix/LedMatrix.hpp"

// default color for power-on plasma effect
#ifndef POWERON_DEFAULT_HUE
#define POWERON_DEFAULT_HUE 170
#endif
#ifndef POWERON_DEFAULT_SAT
#define POWERON_DEFAULT_SAT 255
#endif
#ifndef POWERON_DEFAULT_VAL
#define POWERON_DEFAULT_VAL 255
#endif

class PowerOnAnimation {
public:
    explicit PowerOnAnimation(LedMatrix& m);

    // controller sets progress 0..255 over animation duration
    void setProgress(uint8_t p);
    void setColorHSV(uint8_t h, uint8_t s, uint8_t v);

    void render();

private:
    LedMatrix* matrix;
    uint8_t hue;
    uint8_t sat;
    uint8_t val;
    uint8_t progress; // 0..255
};

#endif // POWERON_ANIMATION_HPP
