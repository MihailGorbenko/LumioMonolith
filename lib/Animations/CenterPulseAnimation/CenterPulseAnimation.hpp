#ifndef CENTER_PULSE_ANIMATION_HPP
#define CENTER_PULSE_ANIMATION_HPP

#include <Arduino.h>
#include "../Animation/Animation.hpp"

#ifndef CENTERPULSE_DEFAULT_HUE
#define CENTERPULSE_DEFAULT_HUE 170 // blue
#endif
#ifndef CENTERPULSE_DEFAULT_SAT
#define CENTERPULSE_DEFAULT_SAT 255
#endif
#ifndef CENTERPULSE_DEFAULT_VAL
#define CENTERPULSE_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class CenterPulseAnimation : public AnimationBase {
public:
    explicit CenterPulseAnimation(LedMatrix& m);

    void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
    void setStride(uint8_t s);       // distance attenuation per row
    void setSpeedDiv(uint8_t div);   // smaller = faster
    void render() override;

private:
    uint8_t stride;  // brightness falloff per row distance from center
    uint8_t speedDiv; // time divider for sin phase
};

#endif // CENTER_PULSE_ANIMATION_HPP
