#include "AppManager.hpp"

#include "../../src/debug.hpp"
#if LOG_ENABLED
#define DBG_PRINT(x) LOG_PRINT(x)
#define DBG_PRINTLN(x) LOG_PRINTLN(x)
#define DBG_PRINTF(...) LOGF("AppManager", __VA_ARGS__)
#else
#define DBG_PRINT(x)
#define DBG_PRINTLN(x)
#define DBG_PRINTF(...)
#endif

// Вспомогательные величины для расчёта дельты энкодера.
static const int ENC_RANGE = (ENC_MAX - ENC_MIN + 1);
static const int ENC_HALF = (ENC_RANGE / 2);

AppManager::AppManager(AnimationManager& am, RotaryEncoder& enc, LedMatrix& m, StorageManager& st)
    : matrix(&m),
      animMgr(&am),
      encoder(&enc),
      storage(&st),
      state(State::Off),
      prevState(State::Off),
      btnDown(false),
      btnStartMs(0),
      lastActivityMs(0),
      lastFrameMs(0),
      frameIntervalMs(0),
      powerOnAnim(m),
      powerOffAnim(m),
      overlayOnActive(false),
      overlayOffActive(false),
            shutdownBeginMs(0),
            shutdownStartProg(255),
            overlayOffProg(255),
      startupBeginMs(0),
      startupLoadedAnim(false),
      startupLoadedMs(0),
      encBaseValue(0),
    brightness(APP_BRIGHTNESS_MIN),
    brightTicks(0),
    colorTicks(0) {
    unsigned long interval = (APP_FPS > 0) ? (1000UL / (unsigned long)APP_FPS) : 33UL;
    if (interval == 0) interval = 1;
    frameIntervalMs = interval;
}

void AppManager::begin() {
    DBG_PRINTLN("[AppManager] Init");
    encBaseValue = 0;

    // Построение гамма-LUT для диапазона 0..255.
    gammaLUT.resize(256);
    for (int i = 0; i < 256; ++i) {
        float norm = (float)i / 255.0f;
        float g = powf(norm, APP_GAMMA) * 255.0f;
        int v = (int)lroundf(g);
        if (v < 0) v = 0; if (v > 255) v = 255;
        gammaLUT[i] = (uint8_t)v;
    }

    loadState();
    applyBrightness();

    // Загрузка конфигурации текущей анимации (если есть).
    if (animMgr) {
        AnimationBase* cur = animMgr->getCurrentAnimation();
        if (cur && storage) storage->loadAnimation(*cur);
    }

    // Инициализация «тиков» из загруженной яркости.
    {
        int range = (int)APP_BRIGHTNESS_MAX - (int)APP_BRIGHTNESS_MIN;
        int b = (int)brightness - (int)APP_BRIGHTNESS_MIN;
        if (b < 0) b = 0; if (b > range) b = range;
        brightTicks = (int)lround(((double)b * (double)APP_BRIGHTNESS_TICKS) / (double)range);
        if (brightTicks < 0) brightTicks = 0;
        if (brightTicks > APP_BRIGHTNESS_TICKS) brightTicks = APP_BRIGHTNESS_TICKS;
    }

    // Старт в состоянии, соответствующем сохранённой конфигурации питания.
    if (appCfg.powerOn) {
        setState(State::Startup);
    } else {
        setState(State::Off);
    }
    lastActivityMs = millis();
}

void AppManager::update() {
    unsigned long now = millis();

    // Обновление состояния энкодера.
    if (encoder) encoder->update();

    // Обновление оверлеев и тайм-аутов переходов.
    updateOverlays(now);

    // Обработка перехода при простое.
    handleIdle(now);

    // Управление частотой рендеринга.
    unsigned long elapsed = now - lastFrameMs;
    bool due = (elapsed >= frameIntervalMs);
    if (due) lastFrameMs = now;

    // Рендерить, когда наступило время (и не Off/Shutdown), активен оверлей или требуется Startup.
    bool canRender = (state != State::Off && state != State::Shutdown);
    if ((due && canRender) || overlayOnActive || overlayOffActive || state == State::Startup) {
        renderFrame();
    }
}

void AppManager::setState(State s) {
    if (state == s) return;
    onExit(state);
    prevState = state;
    state = s;
    onEnter(state);
}

void AppManager::onEnter(State s) {
    switch (s) {
        case State::Off:
            if (matrix) { matrix->powerOff(); }
            overlayOnActive = false; overlayOffActive = false;
            if (animMgr) animMgr->unsetOverlay();
            DBG_PRINTLN("[FSM] Enter OFF");
            break;

        case State::Startup:
            startupBeginMs = millis();
            startupLoadedAnim = false;
            overlayOnActive = true;
            overlayOffActive = false;
            // Пометить состояние питания как включённое и сохранить.
            appCfg.powerOn = 1;
            saveState();
            DBG_PRINTLN("[FSM] Enter STARTUP");
            break;

        case State::Brightness:
            overlayOnActive = false; overlayOffActive = false;
            if (animMgr) animMgr->unsetOverlay();
            // Настройка энкодера для «тиков» яркости (0..APP_BRIGHTNESS_TICKS, без кольца).
            if (encoder) {
                encoder->setBoundaries(0, APP_BRIGHTNESS_TICKS, false);
                encoder->setAccelEnabled(true);
                encoder->setAccelMultipliers(2, 3);
                encoder->setValue(brightTicks);
            }
            encBaseValue = brightTicks;
            DBG_PRINTLN("[FSM] Enter BRIGHTNESS");
            break;

        case State::Animation:
            overlayOnActive = false; overlayOffActive = false;
            if (animMgr) animMgr->unsetOverlay();
            // Один щелчок — одна смена анимации: отключить ускорение.
            if (encoder) {
                encoder->setAccelEnabled(false);
            }
            DBG_PRINTLN("[FSM] Enter ANIMATION");
            break;

        case State::Color:
            overlayOnActive = false; overlayOffActive = false;
            if (animMgr) animMgr->unsetOverlay();
            // Настройка энкодера для «тиков» оттенка (0..APP_COLOR_TICKS, без кольца) с ускорением.
            if (encoder) {
                // derive ticks from current animation hue
                int curTicks = 0;
                if (animMgr) {
                    AnimationBase* cur = animMgr->getCurrentAnimation();
                    if (cur) curTicks = hueToTicks(cur->getConfig().hue);
                }
                colorTicks = curTicks;
                encoder->setBoundaries(0, APP_COLOR_TICKS, false);
                encoder->setAccelEnabled(true);
                encoder->setAccelMultipliers(2, 3);
                encoder->setValue(colorTicks);
                encBaseValue = colorTicks;
            }
            DBG_PRINTLN("[FSM] Enter COLOR");
            break;

        case State::Shutdown:
            // Запускаем плейаут оверлея выключения: продолжаем гасить до конца.
            overlayOnActive = false;
            overlayOffActive = true;
            shutdownBeginMs = millis();
            // Начать с текущего прогресса, накопленного во время удержания.
            shutdownStartProg = overlayOffProg;
            // Пометить состояние питания как выключенное и сохранить.
            appCfg.powerOn = 0;
            saveState();
            DBG_PRINTLN("[FSM] Enter SHUTDOWN");
            break;
    }
}

void AppManager::onExit(State s) {
    switch (s) {
        case State::Brightness:
            // Сохранить конфигурацию приложения при выходе из Brightness.
            saveState();
            // Восстановить границы энкодера на значения по умолчанию с кольцом.
            if (encoder) {
                encoder->setBoundaries(ENC_MIN, ENC_MAX, true);
            }
            break;
        case State::Animation:
            // Сохранить конфигурацию приложения (последний ID анимации) при выходе.
            saveState();
            // Включить ускорение для остальных состояний.
            if (encoder) {
                encoder->setAccelEnabled(true);
            }
            break;
        case State::Color: {
            // Сохранить конфигурацию текущей анимации при выходе.
            if (animMgr) {
                AnimationBase* cur = animMgr->getCurrentAnimation();
                if (cur && storage) storage->saveAnimation(*cur);
            }
            // Восстановить границы энкодера на значения по умолчанию с кольцом.
            if (encoder) {
                encoder->setBoundaries(ENC_MIN, ENC_MAX, true);
            }
            break;
        }
        default:
            break;
    }
}

void AppManager::applyBrightness() {
    if (!matrix) return;
    uint8_t b = brightness;
    if (b < APP_BRIGHTNESS_MIN) b = APP_BRIGHTNESS_MIN;
    if (b > APP_BRIGHTNESS_MAX) b = APP_BRIGHTNESS_MAX;
    uint8_t mapped = b;
    if (!gammaLUT.empty()) mapped = gammaLUT[b];
    matrix->setMasterBrightness(mapped);
}

void AppManager::handleIdle(unsigned long now) {
    if (state == State::Animation || state == State::Color) {
        if ((now - lastActivityMs) >= APP_IDLE_TIMEOUT_MS) {
            // Persist before switching
            if (state == State::Animation) {
                saveState();
            } else if (state == State::Color) {
                if (animMgr) {
                    AnimationBase* cur = animMgr->getCurrentAnimation();
                    if (cur && storage) storage->saveAnimation(*cur);
                }
            }
            setState(State::Brightness);
        }
    }
}

void AppManager::updateOverlays(unsigned long now) {
    // Плейаут оверлея выключения после входа в Shutdown: дожигаем до конца.
    if (state == State::Shutdown) {
        if (overlayOffActive) {
            unsigned long dt = now - shutdownBeginMs;
            // Линейно уменьшаем прогресс до 0 за APP_POWEROFF_OVERLAY_MS
            unsigned long dec = (APP_POWEROFF_OVERLAY_MS > 0) ? ((dt * 255UL) / (unsigned long)APP_POWEROFF_OVERLAY_MS) : 255UL;
            uint8_t prog = (dec >= shutdownStartProg) ? 0 : (uint8_t)(shutdownStartProg - dec);
            powerOffAnim.setProgress(prog);
            overlayOffProg = prog;
            if (prog == 0) {
                overlayOffActive = false;
                setState(State::Off);
            }
        }
        return;
    }

    // Прогресс оверлея запуска и последовательность действий.
    if (state == State::Startup) {
        unsigned long dt = now - startupBeginMs;
        if (dt <= APP_STARTUP_OVERLAY_MS) {
            // Прогресс 0..255 за 3 секунды.
            uint8_t prog = (uint8_t)((dt * 255UL) / (unsigned long)APP_STARTUP_OVERLAY_MS);
            powerOnAnim.setProgress(prog);
            overlayOnActive = true;
        } else {
            overlayOnActive = false;
            if (!startupLoadedAnim) {
                // Загрузить последнюю анимацию по ID или оставить дефолтную.
                if (animMgr && appCfg.lastAnimId != 0) animMgr->setAnimation(appCfg.lastAnimId);
                startupLoadedAnim = true;
                startupLoadedMs = now;
            }
            // Подождать 2 секунды, начать рендер и перейти в Brightness.
            if (startupLoadedAnim && (now - startupLoadedMs) >= APP_STARTUP_RENDER_DELAY_MS) {
                setState(State::Brightness);
            }
        }
    }

    // Оверлей выключения во время удержания кнопки.
    if (btnDown && state != State::Off && state != State::Shutdown) {
        unsigned long held = now - btnStartMs;
        if (held >= APP_POWEROFF_HOLD_THRESHOLD_MS) {
            // Переход в состояние Shutdown.
            setState(State::Shutdown);
            btnDown = false; // consume
        } else if (held >= APP_POWEROFF_OVERLAY_START_MS) {
            // Прогресс оверлея: 2 секунды после 0.5 секунды удержания.
            unsigned long overlayDt = held - APP_POWEROFF_OVERLAY_START_MS;
            if (overlayDt > APP_POWEROFF_OVERLAY_MS) overlayDt = APP_POWEROFF_OVERLAY_MS;
            uint8_t prog = (uint8_t)((overlayDt * 255UL) / (unsigned long)APP_POWEROFF_OVERLAY_MS);
            // Обратный прогресс: 255→0 — визуальное гашение.
            uint8_t negProg = (uint8_t)(255 - prog);
            powerOffAnim.setProgress(negProg);
            overlayOffActive = true;
            overlayOffProg = negProg;
        } else {
            overlayOffActive = false;
        }
    } else {
        overlayOffActive = false;
    }
}

void AppManager::renderFrame() {
    if (!matrix) return;

    // Выбор и установка оверлея.
    if (animMgr) {
        if (overlayOnActive) animMgr->setOverlay(&powerOnAnim);
        else if (overlayOffActive) animMgr->setOverlay(&powerOffAnim);
        else animMgr->unsetOverlay();
    }

    // Рендер в зависимости от состояния.
    bool didRender = false;
    switch (state) {
        case State::Off:
            // Нет рендера и обновления в состоянии Off.
            break;
        case State::Shutdown:
            // В состоянии Shutdown продолжаем рендерить оверлей выключения.
            if (overlayOffActive && animMgr) { animMgr->render(); didRender = true; }
            break;
        case State::Startup:
            if (animMgr) { animMgr->render(); didRender = true; }
            break;
        case State::Brightness:
        case State::Animation:
        case State::Color:
            if (animMgr) { animMgr->render(); didRender = true; }
            break;
    }

    // Вывод кадра только если был рендер.
    if (didRender) matrix->update();
}

void AppManager::onEvent(RotaryEncoder::Event ev, int value) {
    unsigned long now = millis();
    lastActivityMs = now;

    if (ev == RotaryEncoder::PRESS_START) {
        btnDown = true;
        btnStartMs = now;
        return;
    }

    if (ev == RotaryEncoder::PRESS_END) {
        unsigned long held = now - btnStartMs;
        btnDown = false;

        // Из Off: короткое нажатие — переход в Startup.
        if (state == State::Off) {
            if (held < APP_POWEROFF_OVERLAY_START_MS) {
                setState(State::Startup);
            }
            return;
        }

        // Отпускание до порога выключения — возврат в Brightness.
        if (held >= APP_POWEROFF_OVERLAY_START_MS && held < APP_POWEROFF_HOLD_THRESHOLD_MS) {
            setState(State::Brightness);
            return;
        }

        // Длинное удержание обработано в updateOverlays (переход в Shutdown).
        if (state == State::Shutdown) return;

        // Короткое нажатие по кругу: Animation → Color → Brightness → Animation.
        if (held < APP_POWEROFF_OVERLAY_START_MS) {
            if (state == State::Animation) setState(State::Color);
            else if (state == State::Color) setState(State::Brightness);
            else if (state == State::Brightness) setState(State::Animation);
            else setState(State::Brightness);
        }
        return;
    }

    // Rotation
    if (ev == RotaryEncoder::INCREMENT || ev == RotaryEncoder::DECREMENT) {
        // Игнорировать вращение при удержании кнопки.
        if (btnDown) return;

        int delta = value - encBaseValue;
        if (delta > ENC_HALF) delta -= ENC_RANGE;
        else if (delta < -ENC_HALF) delta += ENC_RANGE;
        if (delta == 0) return;
        encBaseValue = value;

        switch (state) {
            case State::Brightness: {
                // Медленное вращение: 30 «тиков» от минимума до максимума; ускорение умножает дельту.
                int newTicks = brightTicks + delta;
                if (newTicks < 0) newTicks = 0;
                if (newTicks > APP_BRIGHTNESS_TICKS) newTicks = APP_BRIGHTNESS_TICKS;
                brightTicks = newTicks;
                brightness = brightnessFromTicks(brightTicks);
                applyBrightness();
                DBG_PRINTF("[Brightness] ticks=%d val=%u\n", brightTicks, (unsigned)brightness);
                break;
            }
            case State::Animation: {
                if (!animMgr || !matrix) break;
                int steps = (delta > 0) ? delta : -delta;
                for (int i = 0; i < steps; ++i) {
                    if (delta > 0) animMgr->switchToNext();
                    else animMgr->switchToPrevious();
                }
                matrix->clear();
                matrix->update();
                AnimationBase* cur = animMgr->getCurrentAnimation();
                if (cur) {
                    appCfg.lastAnimId = cur->getId();
                    DBG_PRINTF("[Animation] switched to id=%u (%s)\n", (unsigned)cur->getId(), cur->getName());
                }
                break;
            }
            case State::Color: {
                if (!animMgr) break;
                int newTicks = colorTicks + delta;
                if (newTicks < 0) newTicks = 0;
                if (newTicks > APP_COLOR_TICKS) newTicks = APP_COLOR_TICKS;
                colorTicks = newTicks;
                uint8_t newHue = hueFromTicks(colorTicks);
                AnimationBase* cur = animMgr->getCurrentAnimation();
                if (cur) cur->setHue(newHue);
                DBG_PRINTF("[Color] ticks=%d hue=%u\n", colorTicks, (unsigned)newHue);
                break;
            }
            case State::Startup:
            case State::Shutdown:
            case State::Off:
                break;
        }
    }
}

bool AppManager::saveState() {
    // Сохранить яркость (0..255) и ID последней анимации.
    appCfg.masterBrightness = (uint16_t)brightness;
    if (animMgr) {
        AnimationBase* cur = animMgr->getCurrentAnimation();
        appCfg.lastAnimId = cur ? cur->getId() : 0;
    } else {
        appCfg.lastAnimId = 0;
    }
    // Зафиксировать текущее состояние питания (не Off/Shutdown => on).
    appCfg.powerOn = (state != State::Off && state != State::Shutdown) ? 1 : 0;
    bool ok = storage ? storage->saveApp(appCfg) : false;
    DBG_PRINTF("[NVS] save app: animId=%u brightness=%u powerOn=%u ok=%d\n", (unsigned)appCfg.lastAnimId, (unsigned)brightness, (unsigned)appCfg.powerOn, (int)ok);
    return ok;
}

bool AppManager::loadState() {
    bool ok = storage ? storage->loadApp(appCfg) : false;
    if (!ok) {
        // Default behavior for first boot with empty storage: power on.
        appCfg.powerOn = 1;
        appCfg.lastAnimId = 0;
    }
    // Клампинг яркости.
    uint16_t b = appCfg.masterBrightness;
    if (b < APP_BRIGHTNESS_MIN) b = APP_BRIGHTNESS_MIN;
    if (b > APP_BRIGHTNESS_MAX) b = APP_BRIGHTNESS_MAX;
    brightness = (uint8_t)b;
    // Обновление «тиков» согласно загруженной яркости.
    {
        int range = (int)APP_BRIGHTNESS_MAX - (int)APP_BRIGHTNESS_MIN;
        int bb = (int)brightness - (int)APP_BRIGHTNESS_MIN;
        if (bb < 0) bb = 0; if (bb > range) bb = range;
        brightTicks = (int)lround(((double)bb * (double)APP_BRIGHTNESS_TICKS) / (double)range);
        if (brightTicks < 0) brightTicks = 0;
        if (brightTicks > APP_BRIGHTNESS_TICKS) brightTicks = APP_BRIGHTNESS_TICKS;
    }
    // Установить анимацию по ID, если доступно.
    if (animMgr && appCfg.lastAnimId != 0) animMgr->setAnimation(appCfg.lastAnimId);
    DBG_PRINTF("[NVS] load app: animId=%u brightness=%u powerOn=%u ok=%d\n", (unsigned)appCfg.lastAnimId, (unsigned)brightness, (unsigned)appCfg.powerOn, (int)ok);
    return ok;
}