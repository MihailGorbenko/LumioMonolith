#include "ColorStaticAnimation.hpp"

ColorStaticAnimation::ColorStaticAnimation(LedMatrix& m)
    : AnimationBase(m, COLORSTATIC_DEFAULT_HUE, COLORSTATIC_DEFAULT_SAT, COLORSTATIC_DEFAULT_VAL) {}

void ColorStaticAnimation::render() {
    if (!matrix) return;
    int w = matrix->width();
    int hgt = matrix->height();
    if (w <= 0) w = 1;
    if (hgt <= 0) hgt = 1;

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < hgt; ++y) {
            matrix->setPixelHSV(x, y, hue, sat, val);
        }
    }
    matrix->show();
}
