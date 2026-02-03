#ifndef ROTARY_ENCODER_HPP
#define ROTARY_ENCODER_HPP
#include <Arduino.h>
#include <stdint.h>

// Encoder pins; can be overridden via build flags (-DROTARY_...=pin).
#ifndef ROTARY_CLK_PIN
#define ROTARY_CLK_PIN 4
#endif
#ifndef ROTARY_DT_PIN
#define ROTARY_DT_PIN 3
#endif
#ifndef ROTARY_SW_PIN
#define ROTARY_SW_PIN 5
#endif

// Button debounce settings; can be overridden via build flags.
#ifndef ROTARY_BTN_DEBOUNCE_MS
#define ROTARY_BTN_DEBOUNCE_MS 50
#endif

// Default encoder boundaries (can be overridden before including this header).
#ifndef ENC_MIN
#define ENC_MIN -32768
#endif
#ifndef ENC_MAX
#define ENC_MAX 32767
#endif

// Threshold values for encoder acceleration (ms).
#ifndef ENCODER_ACCEL_THRESH_SLOW
#define ENCODER_ACCEL_THRESH_SLOW 100
#endif
#ifndef ENCODER_ACCEL_THRESH_FAST
#define ENCODER_ACCEL_THRESH_FAST 40
#endif

class RotaryEncoder {
public:
    enum Event { NONE, PRESS_START, PRESS_END, INCREMENT, DECREMENT };

    // Interface for encoder event listener.
    class IEncoderListener {
    public:
        virtual void onEvent(Event ev, int value) = 0;
    };

    RotaryEncoder();
    void init();
    void update();
    void attachListener(IEncoderListener* l);
    void detachListener(IEncoderListener* l);
    void setValue(int v);
    void setBoundaries(int minV, int maxV, bool wrap);

    // Configure acceleration/speed parameters.
    void setAccelParams(unsigned long med_ms, unsigned long fast_ms, int med_mult, int fast_mult = 3, float filterAlpha = 0.3f);

    // Runtime control of acceleration behavior.
    void setAccelMultipliers(int med_mult, int fast_mult);
    void setAccelThresholds(unsigned long med_ms, unsigned long fast_ms);
    void setAccelEnabled(bool enabled);
    float getVelocity() const;
    // Hard reset of transient encoder context used when switching controller modes.
    void resetContext();

private:
    uint8_t _clkPin, _dtPin, _swPin;
    int _steps;
    int32_t _value;
    int32_t _minV, _maxV;
    bool _wrap;

    static const int MAX_LISTENERS = 4;
    IEncoderListener* _listeners[MAX_LISTENERS];
    int _listenerCount;

    unsigned long _lastSwMillis;
    int _swState;
    bool _btnDown;


    void notify(Event ev, int value);

    // Per-instance quadrature/debounce state (supports multiple encoders).
    uint8_t _lastState;
    int     _accum; // allow accumulation of multiple transitions between updates
    uint8_t _lastRawSw;

    // Velocity/acceleration state and parameters.
    unsigned long _lastStepMillis;
    float _vel;                  // smoothed instantaneous velocity (steps/sec)
    float _velFilterAlpha;       // EMA alpha
    unsigned long _accelMedMs;   // threshold for medium accel (ms)
    unsigned long _accelFastMs;  // threshold for fast accel (ms)
    int _accelMedMult;           // medium multiplier
    int _accelFastMult;          // fast multiplier

    bool _accelEnabled;         // Enable/disable acceleration behavior.

    // Safety: maximum full steps per update (prevents large jumps).
    static const int MAX_FULL_STEPS_PER_UPDATE = 16;
};

#endif // ROTARY_ENCODER_HPP
