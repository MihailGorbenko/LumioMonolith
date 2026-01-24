#ifndef COLOR_STATIC_ANIMATION_HPP
#define COLOR_STATIC_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef COLORSTATIC_DEFAULT_HUE
#define COLORSTATIC_DEFAULT_HUE 0
#endif
#ifndef COLORSTATIC_DEFAULT_SAT
#define COLORSTATIC_DEFAULT_SAT 0
#endif
#ifndef COLORSTATIC_DEFAULT_VAL
#define COLORSTATIC_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class ColorStaticAnimation : public AnimationBase {
public:
    explicit ColorStaticAnimation(LedMatrix& m);
    void render() override;
};

#endif // COLOR_STATIC_ANIMATION_HPP
