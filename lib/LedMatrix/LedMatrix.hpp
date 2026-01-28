#ifndef LED_MATRIX_HPP
#define LED_MATRIX_HPP
#include <FastLED.h>

#define MATRIX_WIDTH 15
#define MATRIX_HEIGHT 5
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
#define DEF_BRIGHTNESS 128
#define LED_PIN 6

/// Класс управления светодиодной матрицей
class LedMatrix {
private:
    CRGB leds[NUM_LEDS];
    CRGB baseLeds[NUM_LEDS]; // буфер оригинальных (незашкалированных) цветов
    int m_width = MATRIX_WIDTH;
    int m_height = MATRIX_HEIGHT;
    uint8_t masterBrightness = DEF_BRIGHTNESS; // Мастер яркость (глобальная)
    int XY(int x, int y);
public:

    LedMatrix();
    void init();
    void clear();
    void update();
    void powerOff();
    int getWidth() const{ return m_width; };
    int getHeight() const{ return m_height; };
    int getNumLeds() const{ return NUM_LEDS; };
    void setMasterBrightness(uint8_t b);
    void setPixelHSV(int x, int y, uint8_t h, uint8_t s, uint8_t v);

};

#endif // LED_MATRIX_HPP