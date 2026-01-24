#ifndef MATRIX_RAIN_ANIMATION_HPP
#define MATRIX_RAIN_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef MATRIXRAIN_DEFAULT_HUE
#define MATRIXRAIN_DEFAULT_HUE 96  // green (classic Matrix style)
#endif
#ifndef MATRIXRAIN_DEFAULT_SAT
#define MATRIXRAIN_DEFAULT_SAT 255
#endif
#ifndef MATRIXRAIN_DEFAULT_VAL
#define MATRIXRAIN_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class MatrixRainAnimation : public AnimationBase {
public:
	explicit MatrixRainAnimation(LedMatrix& m);
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
	void render() override;

private:
	static const int MAX_W = MATRIX_WIDTH;
	uint8_t cols[MAX_W];  // per-column rain position
	uint8_t intensity[MAX_W];  // per-column brightness
};

#endif // MATRIX_RAIN_ANIMATION_HPP
