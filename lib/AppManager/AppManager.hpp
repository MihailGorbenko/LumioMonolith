//#pragma once
#pragma once
#include <Arduino.h>
#include <vector>
#include "../../src/debug.hpp"
#include "../StorageManager/StorageManager.hpp"
#include "../RotaryEncoder/RotaryEncoder.hpp"
#include "../LedMatrix/LedMatrix.hpp"
#include "../Animation/Animation.hpp"
#include "../AnimationManager/AnimationManager.hpp"
#include "../Animations/PowerOffAnimation/PowerOffAnimation.hpp"
#include "../Animations/PowerOnAnimation/PowerOnAnimation.hpp"
#include "AppCfg.hpp"

// Отладка осуществляется через макросы LOG из debug.hpp; локальные флаги не используются.

// Частота кадров (FPS).
#ifndef APP_FPS
#define APP_FPS 30
#endif

// Коэффициент гамма-коррекции.
#ifndef APP_GAMMA
#define APP_GAMMA 2.2f
#endif

// Диапазон мастер-яркости (линейный ввод пользователя).
#ifndef APP_BRIGHTNESS_MIN
#define APP_BRIGHTNESS_MIN 7
#endif
#ifndef APP_BRIGHTNESS_MAX
#define APP_BRIGHTNESS_MAX 255
#endif

// Тайм-ауты и длительности (мс).
#ifndef APP_IDLE_TIMEOUT_MS
#define APP_IDLE_TIMEOUT_MS 30000
#endif
#ifndef APP_STARTUP_OVERLAY_MS
#define APP_STARTUP_OVERLAY_MS 3000
#endif
#ifndef APP_STARTUP_RENDER_DELAY_MS
#define APP_STARTUP_RENDER_DELAY_MS 2000
#endif
#ifndef APP_POWEROFF_OVERLAY_START_MS
#define APP_POWEROFF_OVERLAY_START_MS 500
#endif
#ifndef APP_POWEROFF_OVERLAY_MS
#define APP_POWEROFF_OVERLAY_MS 2000
#endif
#ifndef APP_POWEROFF_HOLD_THRESHOLD_MS
#define APP_POWEROFF_HOLD_THRESHOLD_MS 2500
#endif

// Шаги яркости: полный диапазон за указанное число «тиков» медленного вращения.
#ifndef APP_BRIGHTNESS_TICKS
#define APP_BRIGHTNESS_TICKS 30
#endif

// Шаги оттенка: полный проход по hue за указанное число «тиков».
#ifndef APP_COLOR_TICKS
#define APP_COLOR_TICKS 60
#endif

class AppManager : public RotaryEncoder::IEncoderListener {
public:
    explicit AppManager(AnimationManager& am, RotaryEncoder& enc, LedMatrix& m, StorageManager& st);

    void begin();
    void update();

    // Rotary events
    void onEvent(RotaryEncoder::Event ev, int value) override;

    // Persistence
    bool saveState();
    bool loadState();

    StorageManager* storage;

private:
    // Состояния конечного автомата приложения.
    enum class State { Startup, Shutdown, Animation, Color, Brightness, Off };

    // Внешние зависимости.
    LedMatrix* matrix;
    AnimationManager* animMgr;
    RotaryEncoder* encoder;

    // Текущее состояние автомата.
    State state;
    State prevState;

    // Ввод и таймеры.
    bool btnDown;
    unsigned long btnStartMs;
    unsigned long lastActivityMs;

    // Управление частотой кадров.
    unsigned long lastFrameMs;
    unsigned long frameIntervalMs;

    // Оверлей-анимации включения и выключения.
    PowerOnAnimation powerOnAnim;
    PowerOffAnimation powerOffAnim;
    bool overlayOnActive;
    bool overlayOffActive;

    // Последовательность запуска.
    unsigned long startupBeginMs;
    bool startupLoadedAnim;
    unsigned long startupLoadedMs;

    // Базовое значение энкодера для расчёта дельты.
    int encBaseValue;

    // Яркость (линейный ввод 0..255) с применением гамма-LUT.
    uint8_t brightness;
    std::vector<uint8_t> gammaLUT; // size 256
    int brightTicks; // 0..APP_BRIGHTNESS_TICKS
    int colorTicks;  // 0..APP_COLOR_TICKS

    // Конфигурация приложения.
    AppCfg appCfg;

    // Вспомогательные методы.
    void setState(State s);
    void onEnter(State s);
    void onExit(State s);
    void applyBrightness();
    void handleIdle(unsigned long now);
    void updateOverlays(unsigned long now);
    void renderFrame();
    inline uint8_t brightnessFromTicks(int t) const {
        int tt = t;
        if (tt < 0) tt = 0;
        if (tt > APP_BRIGHTNESS_TICKS) tt = APP_BRIGHTNESS_TICKS;
        int range = (int)APP_BRIGHTNESS_MAX - (int)APP_BRIGHTNESS_MIN;
        int val = (int)APP_BRIGHTNESS_MIN + (int)lround(((double)tt * (double)range) / (double)APP_BRIGHTNESS_TICKS);
        if (val < (int)APP_BRIGHTNESS_MIN) val = (int)APP_BRIGHTNESS_MIN;
        if (val > (int)APP_BRIGHTNESS_MAX) val = (int)APP_BRIGHTNESS_MAX;
        return (uint8_t)val;
    }
    inline uint8_t hueFromTicks(int t) const {
        int tt = t;
        if (tt < 0) tt = 0;
        if (tt > APP_COLOR_TICKS) tt = APP_COLOR_TICKS;
        int val = (int)lround(((double)tt * 255.0) / (double)APP_COLOR_TICKS);
        if (val < 0) val = 0; if (val > 255) val = 255;
        return (uint8_t)val;
    }
    inline int hueToTicks(uint8_t hue) const {
        int h = (int)hue;
        if (h < 0) h = 0; if (h > 255) h = 255;
        int tt = (int)lround(((double)h * (double)APP_COLOR_TICKS) / 255.0);
        if (tt < 0) tt = 0; if (tt > APP_COLOR_TICKS) tt = APP_COLOR_TICKS;
        return tt;
    }
};
