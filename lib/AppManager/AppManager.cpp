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
        : stateChangedThisFrame(false),
            storage(&st),
            pendingState(State::None),
            blockRotationThisFrame(false),
            clickSeq(0),
            handledClickSeq(0),
            fsmLocked(false),
            matrix(&m),
            animMgr(&am),
            encoder(&enc),
            state(State::Off),
            prevState(State::Off),
            btnDown(false),
            btnStartMs(0),
            lastActivityMs(0),
            lastEncoderActivityMs(0),
            lastBtnReleaseMs(0),
            lastBtnPressMs(0),
            lastStateChangeMs(0),
            lastFrameMs(0),
            frameIntervalMs(0),
            powerOnAnim(m),
            powerOffAnim(m),
            overlayOnActive(false),
            overlayOffActive(false),
            shutdownBeginMs(0),
            shutdownStartProg(255),
            overlayOffProg(255),
            overlayOnProg(0),
            lastOverlayOnMs(0),
            startupBeginMs(0),
            startupLoadedAnim(false),
            startupLoadedMs(0),
            encBaseValue(0),
            brightness(APP_BRIGHTNESS_MIN),
            brightTicks(0),
            colorTicks(0),
            savedAppCfgInit(false) {
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

    DBG_PRINTLN("[AppManager] Calling loadState (loadApp)...");
    bool okLoad = loadState();
    DBG_PRINTF("[NVS] load app: animId=%u brightness=%u powerOn=%u ok=%d\n",
               (unsigned)appCfg.lastAnimId, (unsigned)brightness, (unsigned)appCfg.powerOn, (int)okLoad);
    applyBrightness();

    // Зафиксировать начальный снимок сохранённой конфигурации.
    savedAppCfg = appCfg;
    savedAppCfgInit = true;

    // Загрузка конфигурации текущей анимации (если есть) и отметка как чистой.
    if (animMgr) {
        AnimationBase* cur = animMgr->getCurrentAnimation();
        if (cur && storage) {
            bool okAnim = storage->loadAnimation(*cur);
            DBG_PRINTF("[NVS] load anim: id=%u hue=%u ok=%d\n",
                       (unsigned)cur->getId(), (unsigned)cur->getConfig().hue, (int)okAnim);
            cur->clearConfigDirty();
        }
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

    // Сброс флага изменений состояния для текущего кадра
    stateChangedThisFrame = false;

    // Transfer deferred rotation block flag into active block for this update.
    // This ensures we block encoder events in the first update AFTER state change,
    // not in the same update where pendingState was applied.
    blockRotationThisFrame = blockRotationNextFrame;
    blockRotationNextFrame = false;

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

    // Выполнить отложенный переход состояния (только один за кадр)
    if (pendingState != State::None) {
        setState(pendingState);
        // record source of this applied transition and clear pending source
        lastStateChangeSource = pendingStateSource;
        pendingStateSource = StateReqSource::System;
        pendingState = State::None;
        stateChangedThisFrame = true;
        // block rotation for the next processing window (one frame)
        blockRotationNextFrame = true;
    }

    // Allow one click-driven FSM transition per frame only; reset lock here so next update can accept clicks again.
    fsmLocked = false;
}

void AppManager::setState(State s) {
    if (state == s) return;
    DBG_PRINTF("[FSM] setState: %d -> %d\n", (int)state, (int)s);
    onExit(state);
    prevState = state;
    state = s;
    onEnter(state);
    // record the time of the state change to throttle rapid successive requests
    lastStateChangeMs = millis();
    // default source when setState is called directly is System
    lastStateChangeSource = StateReqSource::System;
}

void AppManager::requestState(State s, StateReqSource src) {
    unsigned long now = millis();

    // If this is a user-driven request, enforce guards and canonical ordering.
    if (src == StateReqSource::User) {
        // Throttle requests for a short window after a state change to avoid rapid bouncing.
        if ((now - lastStateChangeMs) < APP_STATE_CHANGE_GUARD_MS) {
            DBG_PRINTF("[FSM] requestState IGNORED GUARD (User): %d -> %d\n", (int)state, (int)s);
            return;
        }

        // Enforce strict ordering for the main cycle: Animation -> Color -> Brightness -> Animation
        auto isCycleState = [](State x) {
            return (x == State::Animation || x == State::Color || x == State::Brightness);
        };
        if (isCycleState(state) && isCycleState(s)) {
            State expected = State::Brightness; // default
            if (state == State::Animation) expected = State::Color;
            else if (state == State::Color) expected = State::Brightness;
            else if (state == State::Brightness) expected = State::Animation;
            if (s != expected) {
                DBG_PRINTF("[FSM] requestState IGNORED ORDER (User): %d -> %d (expected %d)\n", (int)state, (int)s, (int)expected);
                return;
            }
        }
    }

    // If there's already a pending user-driven transition, block system requests until it's applied.
    if (src != StateReqSource::User) {
        if (pendingState != State::None && pendingStateSource == StateReqSource::User) {
            DBG_PRINTF("[FSM] requestState IGNORED PENDING_USER: %d -> %d\n", (int)state, (int)s);
            return;
        }
        if (lastStateChangeSource == StateReqSource::User && (now - lastStateChangeMs) < APP_STATE_CHANGE_GUARD_MS) {
            // Allow overlay-driven finalization to proceed (e.g., shutdown overlay finishing -> Off).
            if (!(src == StateReqSource::Overlay && s == State::Off)) {
                DBG_PRINTF("[FSM] requestState IGNORED GUARD (sys after user): %d -> %d\n", (int)state, (int)s);
                return;
            }
        }
    }

    if (pendingState == State::None && !stateChangedThisFrame) {
        pendingState = s;
        pendingStateSource = src;
        const char* srcName = (src == StateReqSource::User) ? "User" : (src == StateReqSource::Overlay) ? "Overlay" : (src == StateReqSource::Idle) ? "Idle" : "System";
        DBG_PRINTF("[FSM] requestState (%s): %d -> %d\n", srcName, (int)state, (int)s);
    } else {
        DBG_PRINTF("[FSM] requestState IGNORED: %d -> %d\n", (int)state, (int)s);
    }
}

void AppManager::onEnter(State s) {
    // reset encoder transient context on each state entry to avoid carrying _accum/_vel across modes
    if (encoder) encoder->resetContext();

    switch (s) {
        case State::Off:
            if (matrix) { matrix->powerOff(); }
            overlayOnActive = false; overlayOffActive = false;
            if (animMgr) animMgr->unsetOverlay();
            DBG_PRINTLN("[FSM] Enter OFF");
            break;

        case State::Startup:
            startupLoadedAnim = false;
            overlayOnActive = true;
            overlayOffActive = false;
            // Немедленно подготовить оверлей: прогресс 0 и чёрный кадр, чтобы избежать вспышки.
            overlayOnProg = 0;
            powerOnAnim.setProgress(overlayOnProg);
            if (matrix) { matrix->clear(); matrix->update(); }
            // Пометить состояние питания как включённое и сохранить.
            appCfg.powerOn = 1;
            saveState();
            // Старт измерения времени оверлея после потенциально долгих операций сохранения.
            startupBeginMs = millis();
            lastOverlayOnMs = startupBeginMs;
            startupLoadedMs = 0;
            DBG_PRINTLN("[FSM] Enter STARTUP");
            DBG_PRINTF("[Startup] appCfg: animId=%u brightness=%u powerOn=%u\n",
                       (unsigned)appCfg.lastAnimId, (unsigned)brightness, (unsigned)appCfg.powerOn);
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
            // Сохранить конфигурацию текущей анимации при выходе, только если она изменена.
            if (animMgr) {
                AnimationBase* cur = animMgr->getCurrentAnimation();
                if (cur && storage && cur->isConfigDirty()) {
                    if (storage->saveAnimation(*cur)) {
                        cur->clearConfigDirty();
                    }
                }
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
        unsigned long idleDt = (now >= lastEncoderActivityMs) ? (now - lastEncoderActivityMs) : 0UL;
        if (idleDt >= APP_IDLE_TIMEOUT_MS) {
            // Persist before switching
            if (state == State::Animation) {
                saveState();
            } else if (state == State::Color) {
                if (animMgr) {
                    AnimationBase* cur = animMgr->getCurrentAnimation();
                    if (cur && storage && cur->isConfigDirty()) {
                        if (storage->saveAnimation(*cur)) {
                            cur->clearConfigDirty();
                        }
                    }
                }
            }
                // If there is a pending user-driven transition, don't let Idle preempt it.
                if (pendingState != State::None && pendingStateSource == StateReqSource::User) {
                    DBG_PRINTF("[Idle] skip: pending user transition exists (pending=%d)\n", (int)pendingState);
                    return;
                }
                DBG_PRINTF("[Idle] timeout: now=%lu lastEncActivity=%lu dt=%lu -> Brightness\n", now, lastEncoderActivityMs, idleDt);
                requestState(State::Brightness, StateReqSource::Idle);
        }
    }
}

void AppManager::updateOverlays(unsigned long now) {
    // Плейаут оверлея выключения после входа в Shutdown: дожигаем до конца.
    if (state == State::Shutdown) {
        if (overlayOffActive) {
            unsigned long dt = now - shutdownBeginMs;
            unsigned long dec = (APP_POWEROFF_OVERLAY_MS > 0) ? ((dt * 255UL) / (unsigned long)APP_POWEROFF_OVERLAY_MS) : 255UL;
            uint8_t prog = (dec >= shutdownStartProg) ? 0 : (uint8_t)(shutdownStartProg - dec);
            powerOffAnim.setProgress(prog);
            overlayOffProg = prog;
                if (prog == 0) {
                    overlayOffActive = false;
                    requestState(State::Off, StateReqSource::Overlay);
                }
        }
        return;
    }

    // Прогресс оверлея запуска и последовательность действий.
    if (state == State::Startup) {
        // Плавное накопление прогресса с ограничением шага за кадр, без мгновенного перехода на 255.
        unsigned long dtFrame = now - lastOverlayOnMs;
        lastOverlayOnMs = now;
        unsigned long inc = (APP_STARTUP_OVERLAY_MS > 0) ? ((dtFrame * 255UL) / (unsigned long)APP_STARTUP_OVERLAY_MS) : 1UL;
        if (inc == 0UL) inc = 1UL; // гарантировать поступательное увеличение
        if (inc > (unsigned long)APP_STARTUP_MAX_STEP) inc = (unsigned long)APP_STARTUP_MAX_STEP;
        unsigned int next = (unsigned int)overlayOnProg + (unsigned int)inc;
        if (next > 255U) next = 255U;
        overlayOnProg = (uint8_t)next;
        powerOnAnim.setProgress(overlayOnProg);
        overlayOnActive = true;

        // Когда достигли полного прогресса, держим оверлей и выполняем пост-задержку.
        if (overlayOnProg >= 255U) {
            if (!startupLoadedAnim) {
                // Загрузить последнюю анимацию по ID (рендер не начнётся, пока активен оверлей).
                if (animMgr && appCfg.lastAnimId != 0) animMgr->setAnimation(appCfg.lastAnimId);
                startupLoadedAnim = true;
                startupLoadedMs = now;
            }
            // По истечении задержки — снять оверлей и перейти в Brightness.
            if (startupLoadedAnim && (now - startupLoadedMs) >= APP_STARTUP_RENDER_DELAY_MS) {
                overlayOnActive = false;
                requestState(State::Brightness, StateReqSource::Overlay);
            }
        }
    }

    // Оверлей выключения во время удержания кнопки: показываем, но не переходим в Shutdown до отпускания.
    if (btnDown && state != State::Off && state != State::Shutdown) {
        unsigned long held = now - btnStartMs;
        if (held >= APP_POWEROFF_OVERLAY_START_MS) {
            unsigned long overlayDt = held - APP_POWEROFF_OVERLAY_START_MS;
            if (overlayDt > APP_POWEROFF_OVERLAY_MS) overlayDt = APP_POWEROFF_OVERLAY_MS;
            uint8_t prog = (uint8_t)((overlayDt * 255UL) / (unsigned long)APP_POWEROFF_OVERLAY_MS);
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
    // record encoder activity timestamp for idle detection (rotations/presses)
    lastEncoderActivityMs = now;

    // encoder events logging removed to reduce serial noise

    if (ev == RotaryEncoder::PRESS_START) {
        // Защита от дребезга: игнорировать повторные PRESS_START в течение APP_BUTTON_GUARD_MS
        if ((now - lastBtnPressMs) < APP_BUTTON_GUARD_MS) {
            return;
        }
        // Также игнорировать PRESS_START, если только что был RELEASE (ложные повторные клики).
        if ((now - lastBtnReleaseMs) < APP_BUTTON_GUARD_MS) {
            return;
        }
            if (!btnDown) {
                // New click token for this physical press
                ++clickSeq;
                // allow FSM processing for this new click unless explicitly locked later
                fsmLocked = false;
            }
            btnDown = true;
            btnStartMs = now;
            lastBtnPressMs = now;
            // New click token was issued above; clear any pending state transition.
            // Cancel any pending state transition from a предыдущий click
            pendingState = State::None;
            return;
    }

    if (ev == RotaryEncoder::PRESS_END) {
        // Enforce click-sequence token: one physical click -> at most one FSM transition
        if (handledClickSeq == clickSeq) {
            return;
        }

        // Обрабатывать release только если был активный press.
        if (!btnDown) {
            return;
        }
        unsigned long held = now - btnStartMs;

        // Защита от дребезга: игнорировать повторные RELEASE в течение APP_BUTTON_GUARD_MS.
        if ((now - lastBtnReleaseMs) < APP_BUTTON_GUARD_MS) {
            return;
        }
        lastBtnReleaseMs = now;
        btnDown = false;

        // Сохраняем логическое состояние на момент клика, чтобы не зависеть от отложенных переходов
        State logicalState = state;

        // Длинное удержание: подтверждаем переход в Shutdown только при отпускании.
        if (held >= APP_POWEROFF_HOLD_THRESHOLD_MS) {
            if (!fsmLocked) {
                requestState(State::Shutdown);
                // mark this click as handled only when we actually requested a transition
                handledClickSeq = clickSeq;
                fsmLocked = true;
                lastStateChangeMs = now;
            }
            return;
        }

        // Из Off: короткое нажатие — переход в Startup.
            if (logicalState == State::Off) {
            if (held < APP_POWEROFF_OVERLAY_START_MS) {
                if (!fsmLocked) {
                    requestState(State::Startup);
                    handledClickSeq = clickSeq;
                    fsmLocked = true;
                    lastStateChangeMs = now;
                }
            }
            return;
        }

        // Отпускание до порога выключения: не выполняем переход, только снимаем оверлей.
        if (held >= APP_POWEROFF_OVERLAY_START_MS && held < APP_POWEROFF_HOLD_THRESHOLD_MS) {
            // Оверлей выключения будет снят в updateOverlays.
            return;
        }

        // Длинное удержание обработано в updateOverlays (переход в Shutdown).
        if (logicalState == State::Shutdown) return;

        // Короткое нажатие по кругу: Animation → Color → Brightness → Animation.
            if (held < APP_POWEROFF_OVERLAY_START_MS) {
            if ((now - lastStateChangeMs) < APP_STATE_CHANGE_GUARD_MS) {
                return;
            }
            // Always advance to the next state in the cycle, never skip.
            State next = State::Brightness;
            if (logicalState == State::Animation) next = State::Color;
            else if (logicalState == State::Color) next = State::Brightness;
            else if (logicalState == State::Brightness) next = State::Animation;
            if (!fsmLocked) {
                requestState(next);
                handledClickSeq = clickSeq;
                fsmLocked = true;
                lastStateChangeMs = now;
            }
        }
        return;
    }

    // Rotation
    if (ev == RotaryEncoder::INCREMENT || ev == RotaryEncoder::DECREMENT) {
        // Игнорировать вращение при удержании кнопки.
        if (btnDown) return;

        // Ignore one-frame rotation bursts immediately after a state change.
        if (blockRotationThisFrame) return;

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
    // Собрать снимок актуальных значений.
    AppCfg snap;
    snap.masterBrightness = (uint16_t)brightness;
    if (animMgr) {
        AnimationBase* cur = animMgr->getCurrentAnimation();
        snap.lastAnimId = cur ? cur->getId() : 0;
    } else {
        snap.lastAnimId = 0;
    }
    // Текущее состояние питания (не Off/Shutdown => on).
    snap.powerOn = (state != State::Off && state != State::Shutdown) ? 1 : 0;

    // Сохранение только при изменении.
    if (savedAppCfgInit &&
        snap.masterBrightness == savedAppCfg.masterBrightness &&
        snap.lastAnimId == savedAppCfg.lastAnimId &&
        snap.powerOn == savedAppCfg.powerOn) {
        // skip verbose NVS log
        appCfg = snap;
        return true;
    }

    bool ok = storage ? storage->saveApp(snap) : false;
    DBG_PRINTF("[NVS] save app: animId=%u brightness=%u powerOn=%u ok=%d\n",
               (unsigned)snap.lastAnimId, (unsigned)snap.masterBrightness, (unsigned)snap.powerOn, (int)ok);
    if (ok) {
        savedAppCfg = snap;
        savedAppCfgInit = true;
        appCfg = snap;
    }
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