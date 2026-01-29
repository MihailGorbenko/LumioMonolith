#include "PowerOnAnimation.hpp"

PowerOnAnimation::PowerOnAnimation(LedMatrix& m)
    : matrix(&m),
      hue(POWERON_DEFAULT_HUE),
      sat(POWERON_DEFAULT_SAT),
      val(POWERON_DEFAULT_VAL),
      progress(0) {}

void PowerOnAnimation::setProgress(uint8_t p) { progress = p; }

void PowerOnAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
    hue = h; sat = s; val = v;
}

void PowerOnAnimation::render() {
    if (!matrix) return;
    matrix->clear();

        int w = matrix->width();
        int h = matrix->height();
        if (w <= 0) w = 1;
        if (h <= 0) h = 1;

        // Поочерёдно загорающиеся синие сегменты (строки) снизу вверх, без хвоста
        // progress 0..255 -> число полностью зажжённых нижних строк
        uint32_t prod = (uint32_t)progress * (uint32_t)h; // 0..(h*255)
        int litRows = (int)(prod / 255U);                  // полностью зажжённые снизу (логически)

        for (int y = 0; y < h; ++y) {
            // логический индекс снизу: yBottom = (h-1) - y
            int yBottom = (h - 1) - y;
            uint8_t rowV = (yBottom < litRows) ? val : 0;
            if (!rowV) continue;
            for (int x = 0; x < w; ++x) {
                matrix->setPixelHSV(x, y, hue, sat, rowV);
            }
        }

        // show() is managed by AppManager
}
