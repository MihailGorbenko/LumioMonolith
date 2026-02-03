#include "PowerOnAnimation.hpp"

PowerOnAnimation::PowerOnAnimation(LedMatrix& m)
    : OverlayAnimation(m, POWERON_DEFAULT_HUE, POWERON_DEFAULT_SAT, POWERON_DEFAULT_VAL) {}

void PowerOnAnimation::setProgress(uint8_t p) { progress = p; }



void PowerOnAnimation::render() {
    matrix.clear();

    int w = matrix.getWidth();
    int h = matrix.getHeight();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    // Плавное загорание строк снизу вверх:
    // progress 0..255 -> полностью зажжённые нижние строки + частично зажигаем следующую.
    uint32_t prod = (uint32_t)progress * (uint32_t)h; // 0..(h*255)
    int litRows = (int)(prod / 255U);                 // полностью зажжённые снизу (логически)
    int rem = (int)(prod % 255U);                     // доля текущей зажигаемой строки

    for (int y = 0; y < h; ++y) {
        // логический индекс снизу: yBottom = (h-1) - y
        int yBottom = (h - 1) - y;
        uint8_t rowV;
        if (yBottom < litRows) {
            rowV = val; // полностью зажжённые строки
        } else if (yBottom == litRows) {
            // текущая строка загорается плавно
            rowV = (uint8_t)((uint32_t)rem * (uint32_t)val / 255U);
        } else {
            rowV = 0; // выше — ещё тёмно
        }

        if (rowV == 0) continue;
        for (int x = 0; x < w; ++x) {
            matrix.setPixelHSV(x, y, hue, sat, rowV);
        }
    }

}
