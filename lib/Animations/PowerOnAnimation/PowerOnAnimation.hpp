#ifndef POWERON_ANIMATION_HPP
#define POWERON_ANIMATION_HPP

#include <Arduino.h>
#include "../LedMatrix/LedMatrix.hpp"
#include "../Animation/OverlayAnimation.hpp"

// default color for power-on plasma effect
#ifndef POWERON_DEFAULT_HUE
#define POWERON_DEFAULT_HUE 160
#endif
#ifndef POWERON_DEFAULT_SAT
#define POWERON_DEFAULT_SAT 255
#endif
#ifndef POWERON_DEFAULT_VAL
#define POWERON_DEFAULT_VAL 255
#endif

class PowerOnAnimation : public OverlayAnimation {
public:
    explicit PowerOnAnimation(LedMatrix& m);

    // controller sets progress 0..255 over animation duration
    void setProgress(uint8_t p);

    void render() override;

};

#endif // POWERON_ANIMATION_HPP
