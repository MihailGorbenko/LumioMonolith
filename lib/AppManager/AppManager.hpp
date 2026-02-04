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

// Config defaults
#ifndef APP_FPS
#define APP_FPS 30
#endif

#ifndef APP_GAMMA
#define APP_GAMMA 2.2f
#endif

#ifndef APP_BRIGHTNESS_MIN
#define APP_BRIGHTNESS_MIN 15
#endif
#ifndef APP_BRIGHTNESS_MAX
#define APP_BRIGHTNESS_MAX 255
#endif

#ifndef APP_IDLE_TIMEOUT_MS
#define APP_IDLE_TIMEOUT_MS 60000
#endif
#ifndef APP_STARTUP_OVERLAY_MS
#define APP_STARTUP_OVERLAY_MS 3000
#endif
#ifndef APP_STARTUP_RENDER_DELAY_MS
#define APP_STARTUP_RENDER_DELAY_MS 500
#endif
#ifndef APP_STARTUP_MAX_STEP
#define APP_STARTUP_MAX_STEP 16
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

#ifndef APP_BUTTON_GUARD_MS
#define APP_BUTTON_GUARD_MS 600
#endif

#ifndef APP_STATE_CHANGE_GUARD_MS
#define APP_STATE_CHANGE_GUARD_MS 250
#endif

#ifndef APP_BRIGHTNESS_TICKS
#define APP_BRIGHTNESS_TICKS 30
#endif

#ifndef APP_COLOR_TICKS
#define APP_COLOR_TICKS 60
#endif

class AppManager : public RotaryEncoder::IEncoderListener {
    bool stateChangedThisFrame = false;
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
    // --- FSM ---
    enum class State { None, Startup, Shutdown, Animation, Color, Brightness, Off };
    enum class StateReqSource { User, Overlay, Idle, System };

    State pendingState = State::None;
    StateReqSource pendingStateSource = StateReqSource::System;

    // Click/rotation protections
    // blockRotationThisFrame: ignore encoder rotation for the current update frame
    // blockRotationNextFrame: set to block rotation on the next update frame
    bool blockRotationThisFrame = false;
    bool blockRotationNextFrame = false;
    // click sequence tokens to ensure one logical click -> at most one FSM transition
    uint32_t clickSeq = 0;
    uint32_t handledClickSeq = 0;
    // fsmLocked prevents multiple transitions from the same click token
    bool fsmLocked = false;

    void requestState(State s, StateReqSource src = StateReqSource::User);

    // --- External dependencies ---
    LedMatrix* matrix;
    AnimationManager* animMgr;
    RotaryEncoder* encoder;

    // --- FSM state ---
    State state;
    State prevState;

    // --- Input / debounce / click timing ---
    // All timestamp fields use unsigned long (Arduino millis()) and should be
    // used with wrap-safe subtraction (now - prev) to compute intervals.
    bool btnDown;
    unsigned long btnStartMs;
    unsigned long lastEncoderActivityMs;
    unsigned long lastBtnReleaseMs;
    unsigned long lastBtnPressMs;
    unsigned long lastStateChangeMs;
    StateReqSource lastStateChangeSource = StateReqSource::System;

    // --- Rendering / timing ---
    unsigned long lastFrameMs;
    unsigned long frameIntervalMs;

    // --- Overlay animations & shutdown ---
    PowerOnAnimation powerOnAnim;
    PowerOffAnimation powerOffAnim;
    bool overlayOnActive;
    bool overlayOffActive;
    // Timestamp when shutdown overlay began (millis())
    unsigned long shutdownBeginMs;
    // starting progress value when entering Shutdown (0..255)
    uint8_t shutdownStartProg;
    // current progress values for overlays (0..255)
    uint8_t overlayOffProg;
    uint8_t overlayOnProg;
    // last overlay progress update time (millis())
    unsigned long lastOverlayOnMs;

    // --- Startup sequence ---
    // Timestamps in milliseconds for startup overlay sequencing
    unsigned long startupBeginMs;
    bool startupLoadedAnim;
    unsigned long startupLoadedMs;

    // --- Encoder base/value ---
    // encBaseValue holds the raw encoder position base used to compute deltas
    int encBaseValue;

    // --- Brightness / color ---
    uint8_t brightness;
    std::vector<uint8_t> gammaLUT; // size 256
    // brightness and color ticks are small integers used for UI sliders
    int brightTicks; // 0..APP_BRIGHTNESS_TICKS
    int colorTicks;  // 0..APP_COLOR_TICKS

    // --- Persistence / config ---
    AppCfg appCfg;
    AppCfg savedAppCfg;     // last persisted snapshot of app configuration
    bool savedAppCfgInit;   // indicates savedAppCfg is initialized
    bool configDirty{false};

    // Persistence helpers for AppManager
    // mark configuration dirty (needs to be saved later)
    void setConfigDirty();
    // clear dirty flag after a successful save
    void clearConfigDirty();

    // --- Helpers ---
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
