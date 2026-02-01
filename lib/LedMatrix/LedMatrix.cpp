#include "LedMatrix.hpp"


LedMatrix::LedMatrix() {
}


void LedMatrix::init() {
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 3000); // Ограничение по питанию.
    FastLED.setDither(false);                         // Отключаем диффузию, чтобы избежать вспышек при старте.
    FastLED.setBrightness(0);                         // Стартуем с нулевой яркости, чтобы исключить «вспышку».
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    // Немедленно погасить все диоды.
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    // Вернуть системную яркость FastLED к 255: далее используем собственный masterBrightness.
    FastLED.setBrightness(255);
    // Очистить буферы и отобразить чёрный кадр.
    clear();
    update();
}

void LedMatrix::clear() {
    fill_solid(baseLeds, NUM_LEDS, CRGB::Black); // Очищаем буфер исходных цветов.
    fill_solid(leds, NUM_LEDS, CRGB::Black);
}

// fadeAll and clearPixel were removed as unused helpers; keep implementation minimal.

void LedMatrix::update() {
    FastLED.show();
}


void LedMatrix::setPixelHSV(int x, int y, uint8_t h, uint8_t s, uint8_t v) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return;
    }
    int index = XY(x, y);
    if (index < 0 || index >= NUM_LEDS) {
        return;  // Дополнительная проверка: XY может вернуть некорректный индекс.
    }
    // Сохраняем исходный цвет в RGB, затем масштабируем копию для вывода.
    CRGB orig = CHSV(h, s, v);
    baseLeds[index] = orig;
    CRGB out = orig;
    out.nscale8_video(masterBrightness); // Сохраняет пропорции каналов — насыщенность не «выгорает».
    leds[index] = out;
}


void LedMatrix::powerOff() {
    clear();
    update();
}


void LedMatrix::setMasterBrightness(uint8_t b) {
    // Приводим вход к диапазону 0..255.
    uint8_t newB = (uint8_t)constrain(b, 0, 255);

    if (newB == this->masterBrightness) {
        return;
    }

    this->masterBrightness = newB;
    // Пересчитываем все выводимые цвета из baseLeds с новым масштабом.
    for (int i = 0; i < NUM_LEDS; ++i) {
        CRGB out = baseLeds[i];
        out.nscale8_video(this->masterBrightness);
        leds[i] = out;
    }
    update(); // Показываем текущее состояние с новой яркостью.
}

int LedMatrix::XY(int x, int y) {
    // Преобразование координат (x, y) в индекс массива.
    // Компоновка: 5 горизонтальных строк по 15 диодов, змейкой снизу вверх.
    // Первая (нижняя) строка — слева направо; следующая выше — справа налево.
    // Логический origin — сверху слева (y=0), поэтому вычисляем физический индекс строки от низа.
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return -1; // Безопасный возврат при некорректных координатах.
    }
    int physRow = (m_height - 1 - y); // 0 — нижняя строка.
    int rowBase = physRow * m_width;
    if ((physRow & 1) == 0) { // Чётная строка от низа: слева→справа.
        return rowBase + x;
    } else { // Нечётная строка от низа: справа→слева.
        return rowBase + (m_width - 1 - x);
    }
}
