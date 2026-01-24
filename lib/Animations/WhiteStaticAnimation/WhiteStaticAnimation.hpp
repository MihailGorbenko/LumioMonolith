#ifndef WHITE_STATIC_ANIMATION_HPP
#define WHITE_STATIC_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef WHITESTATIC_DEFAULT_HUE
#define WHITESTATIC_DEFAULT_HUE 0
#endif
#ifndef WHITESTATIC_DEFAULT_SAT
#define WHITESTATIC_DEFAULT_SAT 0 // force white
#endif
#ifndef WHITESTATIC_DEFAULT_VAL
#define WHITESTATIC_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class WhiteStaticAnimation : public AnimationBase {
public:
	explicit WhiteStaticAnimation(LedMatrix& m);
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
	void render() override;
};

#endif // WHITE_STATIC_ANIMATION_HPP
