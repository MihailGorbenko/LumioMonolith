#include "WhiteStaticAnimation.hpp"

WhiteStaticAnimation::WhiteStaticAnimation(LedMatrix& m)
	: AnimationBase(m, WHITESTATIC_DEFAULT_HUE, WHITESTATIC_DEFAULT_SAT, WHITESTATIC_DEFAULT_VAL) {}

void WhiteStaticAnimation::setColorHSV(uint8_t, uint8_t, uint8_t) {
	// Always keep pure white internally; brightness is global via masterBrightness
	hue = 0;
	sat = 0;
	val = WHITESTATIC_DEFAULT_VAL;
}

void WhiteStaticAnimation::render() {
	if (!matrix) return;
	int w = matrix->width();
	int hgt = matrix->height();
	if (w <= 0) w = 1;
	if (hgt <= 0) hgt = 1;

	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < hgt; ++y) {
			matrix->setPixelHSV(x, y, 0, 0, val);
		}
	}
	matrix->show();
}
