#ifndef REACTOR_TURBINES_ANIMATION_HPP
#define REACTOR_TURBINES_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef REACTOR_DEFAULT_HUE
#define REACTOR_DEFAULT_HUE 160
#endif

#ifndef REACTOR_DEFAULT_SAT
#define REACTOR_DEFAULT_SAT 255
#endif

#ifndef REACTOR_DEFAULT_VAL
#define REACTOR_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

#ifndef REACTOR_SEGMENT_LEN
#define REACTOR_SEGMENT_LEN 7
#endif

#ifndef REACTOR_SPEED_MS
#define REACTOR_SPEED_MS 80
#endif

#ifndef REACTOR_DIM_SCALE
#define REACTOR_DIM_SCALE 160 // scale8(val, 160) ~ 62% brightness for the dim turbine
#endif

class ReactorTurbinesAnimation : public AnimationBase {
public:
    explicit ReactorTurbinesAnimation(LedMatrix& m);
    void render() override;
};

#endif // REACTOR_TURBINES_ANIMATION_HPP
