#include "LedMatrix.hpp"


LedMatrix::LedMatrix() {
}


void LedMatrix::init() {
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 3000); // Power limit.
    FastLED.setDither(false);                         // Disable dithering to avoid startup flicker.
    FastLED.setBrightness(0);                         // Start with zero brightness to prevent a flash.
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    // Immediately clear all LEDs.
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    // Restore FastLED system brightness to 255; further brightness is controlled by masterBrightness.
    FastLED.setBrightness(255);
    // Clear buffers and display a black frame.
    clear();
    update();
}

void LedMatrix::clear() {
    fill_solid(baseLeds, NUM_LEDS, CRGB::Black); // Clear base color buffer.
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
        return;  // Additional check: XY may return an invalid index.
    }
    // Save the original color in RGB, then scale a copy for output.
    CRGB orig = CHSV(h, s, v);
    baseLeds[index] = orig;
    CRGB out = orig;
    out.nscale8_video(masterBrightness); // Preserve channel proportions — saturation won't wash out.
    leds[index] = out;
}


void LedMatrix::powerOff() {
    clear();
    update();
}


void LedMatrix::setMasterBrightness(uint8_t b) {
    // Clamp input to range 0..255.
    uint8_t newB = (uint8_t)constrain(b, 0, 255);

    if (newB == this->masterBrightness) {
        return;
    }

    this->masterBrightness = newB;
    // Recompute all output colors from baseLeds using the new scale.
    for (int i = 0; i < NUM_LEDS; ++i) {
        CRGB out = baseLeds[i];
        out.nscale8_video(this->masterBrightness);
        leds[i] = out;
    }
    update(); // Показываем текущее состояние с новой яркостью.
}

int LedMatrix::XY(int x, int y) {
    // Convert (x, y) coordinates to array index.
    // Layout: 5 horizontal rows of 15 LEDs, wired in a serpentine pattern bottom-to-top.
    // The first (bottom) row is left-to-right; the next row above is right-to-left.
    // Logical origin is top-left (y=0), so compute the physical row from the bottom.
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return -1; // Safe return for invalid coordinates.
    }
    int physRow = (m_height - 1 - y); // 0 is the bottom row.
    int rowBase = physRow * m_width;
    if ((physRow & 1) == 0) { // Even row from bottom: left→right.
        return rowBase + x;
    } else { // Нечётная строка от низа: справа→слева.
        return rowBase + (m_width - 1 - x); // Odd row from bottom: right→left.
    }
}
