#include "FlameColumnsAnimation.hpp"
#include <FastLED.h>

FlameColumnsAnimation::FlameColumnsAnimation(LedMatrix& m)
        : AnimationBase(m, FLAME_DEFAULT_HUE, FLAME_DEFAULT_SAT, FLAME_DEFAULT_VAL),
            cooling(10), sparking(200), speedDiv(3), nextStepMs(0), stepPeriodMs(40) {
    // init heat buffer
    for (int x = 0; x < MATRIX_WIDTH; ++x) {
        for (int y = 0; y < MATRIX_HEIGHT; ++y) {
            heat[x][y] = 0;
        }
    }
}

void FlameColumnsAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
    AnimationBase::setColorHSV(h, s, v);
}

void FlameColumnsAnimation::setCooling(uint8_t c) { cooling = c; }
void FlameColumnsAnimation::setSparking(uint8_t s) { sparking = s; }
void FlameColumnsAnimation::setSpeedDiv(uint8_t div) { speedDiv = (div == 0) ? 1 : div; }

void FlameColumnsAnimation::heatStep(int w, int h) {
    // cool down every cell slightly
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            uint8_t cool = random8(0, cooling + 1);
            heat[x][y] = (heat[x][y] > cool) ? (heat[x][y] - cool) : 0;
        }
    }

    // diffuse heat upward along columns (bottom to top)
    for (int x = 0; x < w; ++x) {
        for (int y = h - 1; y >= 1; --y) {
            uint16_t sum = heat[x][y] + heat[x][y - 1];
            heat[x][y] = (uint8_t)min<uint16_t>(255, (sum * 3) / 4); // mild diffusion with decay
        }
    }

    // maintain a warm baseline at the bottom and add random sparks
    for (int x = 0; x < w; ++x) {
        // baseline so columns never fully dark
        heat[x][0] = max<uint8_t>(heat[x][0], 40);
        if (random8() < sparking) {
            heat[x][0] = qadd8(heat[x][0], random8(120, 255));
        }
    }
}

void FlameColumnsAnimation::drawFlame(int w, int h) {
    matrix->clear();
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            uint8_t temperature = heat[x][y];
            // Map heat to flame hue gradient: red->orange->yellow
            uint8_t flameHue = FLAME_DEFAULT_HUE + scale8(temperature, 40);
            uint8_t brightness = scale8(val, temperature); // brighter with heat
            // draw using logical coordinates; tongues rise vertically per column
            matrix->setPixelHSV(x, y, flameHue, sat, brightness);
        }
    }
}

void FlameColumnsAnimation::render() {
    if (!matrix) return;
    int w = matrix->width();
    int h = matrix->height();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    uint32_t now = millis();
    if (now >= nextStepMs) {
        nextStepMs = now + (stepPeriodMs * speedDiv);
        heatStep(w, h);
    }

    drawFlame(w, h);
    matrix->show();
}
