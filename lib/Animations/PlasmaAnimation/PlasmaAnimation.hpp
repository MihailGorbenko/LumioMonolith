#ifndef PLASMA_ANIMATION_HPP
#define PLASMA_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef PLASMA_DEFAULT_HUE
#define PLASMA_DEFAULT_HUE 160
#endif
#ifndef PLASMA_DEFAULT_SAT
#define PLASMA_DEFAULT_SAT 255
#endif
#ifndef PLASMA_DEFAULT_VAL
#define PLASMA_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class PlasmaAnimation : public AnimationBase {
public:
	explicit PlasmaAnimation(LedMatrix& m);
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
	void render() override;
};

#endif // PLASMA_ANIMATION_HPP
