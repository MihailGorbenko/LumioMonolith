#include "MatrixCodeRainAnimation.hpp"
#include <FastLED.h>

MatrixCodeRainAnimation::MatrixCodeRainAnimation(uint16_t id)
    : AnimationBase(MATRIX_RAIN_DEFAULT_HUE, id),
        tailLen(1), numCols(0), numRows(0), nextStepMs(0), stepPeriodMs(50) {
    int w = 0;
    int h = 0;
    // обычная логика: колонки = ширина, строки = высота
    numCols = w;
    numRows = h;
    // хвост ограничиваем высотой (движение по Y)
    tailLen = (uint8_t)min<uint8_t>(tailLen, (numRows > 0 ? (numRows - 1) : 1));
    // выделяем память для массивов по количеству колонок (15)
    heads.resize(numCols);
    counter.resize(numCols);
    speeds.resize(numCols);
    // головы по каждому столбцу X (позиции вдоль сегмента) с разными начальными позициями и скоростями
        for (int x = 0; x < numCols; ++x) {
        heads[x] = random8(0, (numRows > 0 ? numRows : 1));
        // per-column random speed: larger -> slower. Choose 4..12 (slower than before)
        speeds[x] = (uint8_t)random8(4, 13);
        // стартовый сдвиг в пределах скорости, чтобы стартовали в разное время
        counter[x] = random8(speeds[x]);
    }
}

MatrixCodeRainAnimation::~MatrixCodeRainAnimation() {
    // vectors automatically cleaned up
}

void MatrixCodeRainAnimation::setTailLen(uint8_t len) {
    uint8_t maxTail = (numRows > 0) ? (numRows - 1) : 1;
    tailLen = (len == 0) ? 1 : (uint8_t)min<uint8_t>(len, maxTail);
}

void MatrixCodeRainAnimation::render(LedMatrix& m) {
    int w = m.getWidth();
    int h = m.getHeight();
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    uint32_t now = millis();
    if ((int32_t)(now - nextStepMs) >= 0) {
        nextStepMs = now + stepPeriodMs;
        // движение вниз по Y с индивидуальным таймингом для каждой второй колонки (через столбец)
        for (int x = 0; x < numCols; x += 2) {
            counter[x]++;
            if (counter[x] >= speeds[x]) {
                counter[x] = 0;
                int head = heads[x] + 1;
                if (head >= numRows) head = 0;
                heads[x] = head;
            }
        }
    }

    m.clear();
    // вертикальный дождь сверху вниз: капли через столбец (чистый цвет)
    for (int x = 0; x < numCols; x += 2) {
        int head = heads[x];
        for (int t = 0; t <= tailLen; ++t) {
            int y = head - t;
            if (y < 0) y += numRows; // wrap around
            if (y >= numRows) continue;
            uint8_t vpix = (t == 0) ? ANIMATION_DEFAULT_VAL : scale8(ANIMATION_DEFAULT_VAL, 180); // slight brightness gap for tail
            m.setPixelHSV(x, y, animCfg.hue, ANIMATION_DEFAULT_SAT, vpix);
        }
    }

    // show() is managed by AppController
}

// Base class provides ISerializable
