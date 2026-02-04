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

// Helper constants for computing encoder delta.
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

    // Build gamma LUT for range 0..255.
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
    DBG_PRINTF("[NVS] load app: brightness=%u powerOn=%u ok=%d\n",
               (unsigned)brightness, (unsigned)appCfg.powerOn, (int)okLoad);
    applyBrightness();

    // Capture initial snapshot of loaded configuration.
    savedAppCfg = appCfg;
    savedAppCfgInit = true;

    // AnimationManager now loads/activates current animation in its init().

    // Initialize tick counters from loaded brightness.
    {
        int range = (int)APP_BRIGHTNESS_MAX - (int)APP_BRIGHTNESS_MIN;
        int b = (int)brightness - (int)APP_BRIGHTNESS_MIN;
        if (b < 0) b = 0; if (b > range) b = range;
        brightTicks = (int)lround(((double)b * (double)APP_BRIGHTNESS_TICKS) / (double)range);
        if (brightTicks < 0) brightTicks = 0;
        if (brightTicks > APP_BRIGHTNESS_TICKS) brightTicks = APP_BRIGHTNESS_TICKS;
    }

    // Start in state corresponding to saved power configuration.
    if (appCfg.powerOn) {
        setState(State::Startup);
    } else {
        setState(State::Off);
    }
    // Initialize encoder activity timestamp to now to avoid treating startup as idle
    lastEncoderActivityMs = millis();
}

void AppManager::update() {
    unsigned long now = millis();

    // Clear state-changed flag for this frame
    stateChangedThisFrame = false;

    // Transfer deferred rotation block flag into active block for this update.
    // This ensures we block encoder events in the first update AFTER state change,
    // not in the same update where pendingState was applied.
    blockRotationThisFrame = blockRotationNextFrame;
    blockRotationNextFrame = false;

    // Update encoder state.
    if (encoder) encoder->update();

    // Update overlays and transition timeouts.
    updateOverlays(now);

    // Handle idle-triggered transitions.
    handleIdle(now);

    // Autosave app config: if configDirty and no encoder activity for idle timeout, persist appCfg.
    if (configDirty) {
        unsigned long idleDtEnc = (now >= lastEncoderActivityMs) ? (now - lastEncoderActivityMs) : 0UL;
        if (idleDtEnc >= APP_IDLE_TIMEOUT_MS) {
            DBG_PRINTF("[Autosave] saving appCfg after %lu ms idle\n", idleDtEnc);
            saveState();
            clearConfigDirty();
        }
    }

    // Run animation manager periodic tasks independent of render FPS
    if (animMgr) animMgr->update();

    // Control render frequency.
    unsigned long elapsed = now - lastFrameMs;
    bool due = (elapsed >= frameIntervalMs);
    if (due) lastFrameMs = now;

    // Render when due (and not Off/Shutdown), or when overlay/startup is active.
    bool canRender = (state != State::Off && state != State::Shutdown);
    if ((due && canRender) || overlayOnActive || overlayOffActive || state == State::Startup) {
        renderFrame();
    }

    // Apply a pending state transition (at most one per frame)
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

    // Allow one click-driven FSM transition per frame only; reset lock here so next update can accept clicks.
    fsmLocked = false;
}

void AppManager::setState(State s) {
    if (state == s) return;
    DBG_PRINTF("[FSM] setState: %d -> %d\n", (int)state, (int)s);
    onExit(state);
    prevState = state;
    state = s;
    onEnter(state);
    // Record the time of the state change to throttle rapid successive requests
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
    // Reset encoder transient context on each state entry to avoid carrying _accum/_vel across modes
    if (encoder) encoder->resetContext();

    switch (s) {
        case State::Off:
            if (matrix) { matrix->powerOff(); }
            overlayOnActive = false; overlayOffActive = false;
            if (animMgr) animMgr->unsetOverlay();
            DBG_PRINTLN("[FSM] Enter OFF");
            // Ensure animations are persisted on power-off even if app config isn't dirty.
            if (animMgr) animMgr->forceSave();
            // If application config changed, persist it as well.
            if (configDirty) {
                saveState();
                clearConfigDirty();
            }
            break;

        case State::Startup:
            startupLoadedAnim = false;
            overlayOnActive = true;
            overlayOffActive = false;
            // Prepare overlay immediately: progress 0 and a black frame to avoid flash.
            overlayOnProg = 0;
            powerOnAnim.setProgress(overlayOnProg);
            if (matrix) { matrix->clear(); matrix->update(); }
            // Mark power state as ON and mark config dirty.
            appCfg.powerOn = 1;
            setConfigDirty();
            // Start measuring overlay time after potentially long save operations.
            startupBeginMs = millis();
            lastOverlayOnMs = startupBeginMs;
            startupLoadedMs = 0;
            DBG_PRINTLN("[FSM] Enter STARTUP");
            DBG_PRINTF("[Startup] appCfg: brightness=%u powerOn=%u\n",
                       (unsigned)brightness, (unsigned)appCfg.powerOn);
            break;

        case State::Brightness:
            overlayOnActive = false; overlayOffActive = false;
            if (animMgr) animMgr->unsetOverlay();
            // Configure encoder for brightness ticks (0..APP_BRIGHTNESS_TICKS, no wrap).
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
            // One click -> one animation change: disable acceleration.
            if (encoder) {
                encoder->setAccelEnabled(false);
            }
            DBG_PRINTLN("[FSM] Enter ANIMATION");
            break;

        case State::Color:
            overlayOnActive = false; overlayOffActive = false;
            if (animMgr) animMgr->unsetOverlay();
            // Configure encoder for hue ticks (0..APP_COLOR_TICKS, no wrap) with acceleration.
            if (encoder) {
                // derive ticks from current animation hue via AnimationManager
                int curTicks = 0;
                if (animMgr) {
                    uint8_t curHue = animMgr->getCurrentHue();
                    curTicks = hueToTicks(curHue);
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
            // Start shutdown overlay playout: continue fading until complete.
            overlayOnActive = false;
            overlayOffActive = true;
            shutdownBeginMs = millis();
            // Start from current progress accumulated during the hold.
            shutdownStartProg = overlayOffProg;
            // Mark power state as OFF and mark config dirty.
            appCfg.powerOn = 0;
            setConfigDirty();
            DBG_PRINTLN("[FSM] Enter SHUTDOWN");
            break;
    }
}

void AppManager::onExit(State s) {
    switch (s) {
        case State::Brightness:
            // If master brightness was changed, update appCfg and mark dirty.
            if (!savedAppCfgInit || (uint16_t)brightness != savedAppCfg.masterBrightness) {
                appCfg.masterBrightness = (uint16_t)brightness;
                setConfigDirty();
            }
            // Restore encoder boundaries to default with wrap.
            if (encoder) {
                encoder->setBoundaries(ENC_MIN, ENC_MAX, true);
            }
            break;
        case State::Animation:
            // Animation changes (current animation id) are managed and persisted by AnimationManager.
            // Re-enable acceleration for other states.
            if (encoder) {
                encoder->setAccelEnabled(true);
            }
            break;
        case State::Color: {
            // AnimationManager is responsible for saving animation configs; no-op here.
            // Restore encoder boundaries to default with wrap.
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
            // Persist only animation-specific config here; app config autosave is handled globally in update().
            // AnimationManager handles saving animation configs via its own autosave/forceSave.

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
    // Shutdown overlay playout after entering Shutdown: finish remaining fade.
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

    // Startup overlay progress and sequence.
    if (state == State::Startup) {
        // Accumulate progress smoothly with a per-frame step cap to avoid instant jump to 255.
        unsigned long dtFrame = now - lastOverlayOnMs;
        lastOverlayOnMs = now;
        unsigned long inc = (APP_STARTUP_OVERLAY_MS > 0) ? ((dtFrame * 255UL) / (unsigned long)APP_STARTUP_OVERLAY_MS) : 1UL;
        if (inc == 0UL) inc = 1UL; // ensure incremental increase
        if (inc > (unsigned long)APP_STARTUP_MAX_STEP) inc = (unsigned long)APP_STARTUP_MAX_STEP;
        unsigned int next = (unsigned int)overlayOnProg + (unsigned int)inc;
        if (next > 255U) next = 255U;
        overlayOnProg = (uint8_t)next;
        powerOnAnim.setProgress(overlayOnProg);
        overlayOnActive = true;

        // When full progress is reached, hold the overlay and perform the post-delay.
            if (overlayOnProg >= 255U) {
                if (!startupLoadedAnim) {
                    // Let AnimationManager handle its own last-animation restore
                    startupLoadedAnim = true;
                    startupLoadedMs = now;
                }
            // After the post-delay expires, remove overlay and transition to Brightness.
            if (startupLoadedAnim && (now - startupLoadedMs) >= APP_STARTUP_RENDER_DELAY_MS) {
                overlayOnActive = false;
                requestState(State::Brightness, StateReqSource::Overlay);
            }
        }
    }

    // Power-off overlay during button hold: show it, but do not enter Shutdown until release.
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

    // Select and apply the overlay.
    if (animMgr) {
        if (overlayOnActive) animMgr->setOverlay(&powerOnAnim);
        else if (overlayOffActive) animMgr->setOverlay(&powerOffAnim);
        else animMgr->unsetOverlay();
    }

    // Render according to current state.
    bool didRender = false;
    switch (state) {
        case State::Off:
            // No rendering or updates in Off state.
            break;
        case State::Shutdown:
            // In Shutdown state, continue rendering the power-off overlay.
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

    // Update matrix only if a frame was rendered.
    if (didRender) matrix->update();
}

void AppManager::onEvent(RotaryEncoder::Event ev, int value) {
    unsigned long now = millis();
    // record encoder activity timestamp for idle detection (rotations/presses)
    lastEncoderActivityMs = now;

    // encoder events logging removed to reduce serial noise

    if (ev == RotaryEncoder::PRESS_START) {
        // Debounce: ignore repeated PRESS_START within APP_BUTTON_GUARD_MS
        if ((now - lastBtnPressMs) < APP_BUTTON_GUARD_MS) {
            return;
        }
            // Also ignore PRESS_START if a RELEASE occurred recently (spurious repeats).
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
                // Cancel any pending state transition from a previous click
            pendingState = State::None;
            return;
    }

    if (ev == RotaryEncoder::PRESS_END) {
        // Enforce click-sequence token: one physical click -> at most one FSM transition
        if (handledClickSeq == clickSeq) {
            return;
        }

        // Process release only if there was an active press.
        if (!btnDown) {
            return;
        }
        unsigned long held = now - btnStartMs;

        // Debounce: ignore repeated RELEASE within APP_BUTTON_GUARD_MS.
        if ((now - lastBtnReleaseMs) < APP_BUTTON_GUARD_MS) {
            return;
        }
        lastBtnReleaseMs = now;
        btnDown = false;

        // Save logical state at the time of click to avoid depending on deferred transitions
        State logicalState = state;

        // Long hold: confirm the Shutdown transition only on release.
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

        // From Off: short press -> Startup.
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

        // Release before shutdown threshold: do not transition, only remove overlay.
        if (held >= APP_POWEROFF_OVERLAY_START_MS && held < APP_POWEROFF_HOLD_THRESHOLD_MS) {
            // Power-off overlay will be cleared in updateOverlays.
            return;
        }

        // Long hold is handled in updateOverlays (transition to Shutdown).
        if (logicalState == State::Shutdown) return;

        // Short press cycles: Animation → Color → Brightness → Animation.
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
        // Ignore rotation while the button is held.
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
                // Slow rotation: 30 ticks from minimum to maximum; acceleration multiplies the delta.
                int newTicks = brightTicks + delta;
                if (newTicks < 0) newTicks = 0;
                if (newTicks > APP_BRIGHTNESS_TICKS) newTicks = APP_BRIGHTNESS_TICKS;
                brightTicks = newTicks;
                brightness = brightnessFromTicks(brightTicks);
                applyBrightness();
                DBG_PRINTF("[Brightness] ticks=%d val=%u\n", brightTicks, (unsigned)brightness);
                    // Mark app config dirty when brightness differs from last saved value.
                    if (!savedAppCfgInit || (uint16_t)brightness != savedAppCfg.masterBrightness) {
                        setConfigDirty();
                    }
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
                // AnimationManager handles loading/activation of target animations.
                uint16_t id = animMgr ? animMgr->getCurrentId() : 0;
                const char* name = animMgr ? animMgr->getCurrentName() : "";
                DBG_PRINTF("[Animation] switched to id=%u (%s)\n", (unsigned)id, name ? name : "");
                break;
            }
            case State::Color: {
                if (!animMgr) break;
                int newTicks = colorTicks + delta;
                if (newTicks < 0) newTicks = 0;
                if (newTicks > APP_COLOR_TICKS) newTicks = APP_COLOR_TICKS;
                colorTicks = newTicks;
                uint8_t newHue = hueFromTicks(colorTicks);
                if (animMgr) animMgr->setCurrentHue(newHue);
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
    // Assemble snapshot of current values.
    AppCfg snap;
    snap.masterBrightness = (uint16_t)brightness;
    // Current power state (not Off/Shutdown => on).
    snap.powerOn = (state != State::Off && state != State::Shutdown) ? 1 : 0;

    // Save only if changed.
    if (savedAppCfgInit &&
        snap.masterBrightness == savedAppCfg.masterBrightness &&
        snap.powerOn == savedAppCfg.powerOn) {
        appCfg = snap;
        return true;
    }

    bool ok = storage ? storage->saveApp(snap) : false;
    DBG_PRINTF("[NVS] save app: brightness=%u powerOn=%u ok=%d\n",
               (unsigned)snap.masterBrightness, (unsigned)snap.powerOn, (int)ok);
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
    }
    // Clamp brightness.
    uint16_t b = appCfg.masterBrightness;
    if (b < APP_BRIGHTNESS_MIN) b = APP_BRIGHTNESS_MIN;
    if (b > APP_BRIGHTNESS_MAX) b = APP_BRIGHTNESS_MAX;
    brightness = (uint8_t)b;
    // Update ticks according to loaded brightness.
    {
        int range = (int)APP_BRIGHTNESS_MAX - (int)APP_BRIGHTNESS_MIN;
        int bb = (int)brightness - (int)APP_BRIGHTNESS_MIN;
        if (bb < 0) bb = 0; if (bb > range) bb = range;
        brightTicks = (int)lround(((double)bb * (double)APP_BRIGHTNESS_TICKS) / (double)range);
        if (brightTicks < 0) brightTicks = 0;
        if (brightTicks > APP_BRIGHTNESS_TICKS) brightTicks = APP_BRIGHTNESS_TICKS;
    }
    // Let AnimationManager handle any last-animation restore
    DBG_PRINTF("[NVS] load app: brightness=%u powerOn=%u ok=%d\n", (unsigned)brightness, (unsigned)appCfg.powerOn, (int)ok);
    return ok;
}

// Mark that the app configuration was changed and needs persistence.
void AppManager::setConfigDirty() {
    configDirty = true;
}

// Clear dirty flag after configuration has been persisted.
void AppManager::clearConfigDirty() {
    configDirty = false;
}
