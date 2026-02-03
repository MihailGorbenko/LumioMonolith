#include "RotaryEncoder.hpp"
#include <stdint.h>
#include <stdlib.h> // abs(int).

// Modified constructor: default steps = 1.
RotaryEncoder::RotaryEncoder()
        : _clkPin(ROTARY_CLK_PIN), _dtPin(ROTARY_DT_PIN), _swPin(ROTARY_SW_PIN),
            _steps(1),
            _value(0), _minV(ENC_MIN), _maxV(ENC_MAX), _wrap(true),
      _listenerCount(0), _lastSwMillis(0),
      _swState(HIGH), _btnDown(false),
      // init new per-instance state
      _lastState(0), _accum(0), _lastRawSw(HIGH),
      // velocity/accel defaults
      _lastStepMillis(0), _vel(0.0f), _velFilterAlpha(0.3f),
      _accelMedMs(100), _accelFastMs(40), _accelMedMult(2), _accelFastMult(3),
      _accelEnabled(true) {
    for (int i = 0; i < MAX_LISTENERS; ++i) _listeners[i] = nullptr;
}

// Transition table used for quadrature decoding.
static const int8_t TRANS_TABLE[16] = {
    0,  1, -1,  0,
   -1,  0,  0,  1,
    1,  0,  0, -1,
    0, -1,  1,  0
};

void RotaryEncoder::setAccelParams(unsigned long med_ms, unsigned long fast_ms, int med_mult, int fast_mult, float filterAlpha) {
    if (med_ms == 0) med_ms = 1;
    if (fast_ms == 0) fast_ms = 1;
    if (filterAlpha <= 0.0f || filterAlpha > 1.0f) filterAlpha = 0.3f;
    _accelMedMs = med_ms;
    _accelFastMs = fast_ms;
    _accelMedMult = (med_mult > 0) ? med_mult : 2;
    _accelFastMult = (fast_mult > 0) ? fast_mult : 3; // Default fast multiplier = ×3.
    _velFilterAlpha = filterAlpha;
}

// Runtime control of acceleration from the controller.
void RotaryEncoder::setAccelMultipliers(int med_mult, int fast_mult) {
    if (med_mult > 0 && med_mult < 10) _accelMedMult = med_mult;  // reasonable bounds: 1..9
    if (fast_mult > 0 && fast_mult < 10) _accelFastMult = fast_mult;  // reasonable bounds: 1..9
}
void RotaryEncoder::setAccelThresholds(unsigned long med_ms, unsigned long fast_ms) {
    if (med_ms == 0) med_ms = 1;
    if (fast_ms == 0) fast_ms = 1;
    _accelMedMs = med_ms;
    _accelFastMs = fast_ms;
}
void RotaryEncoder::setAccelEnabled(bool enabled) { _accelEnabled = enabled; }
float RotaryEncoder::getVelocity() const { return _vel; }

void RotaryEncoder::init() {
    pinMode(_clkPin, INPUT_PULLUP);
    pinMode(_dtPin, INPUT_PULLUP);
    pinMode(_swPin, INPUT_PULLUP);

    // Initialize the finite-state machine (per instance) for quadrature decoding.
    _lastState = (digitalRead(_clkPin) << 1) | digitalRead(_dtPin);
    _accum = 0;

    // Initialize button debounce state consistently.
    _swState = digitalRead(_swPin);
    _lastRawSw = _swState;
    _btnDown = (_swState == LOW);
    _lastSwMillis = millis();

    // Reset timing stamps for step/velocity.
    _lastStepMillis = 0;
    _vel = 0.0f;
}

void RotaryEncoder::resetContext() {
    _accum = 0;
    _lastStepMillis = 0;
    _vel = 0.0f;
}

void RotaryEncoder::setValue(int v) {
    if (v < _minV) v = _minV;
    if (v > _maxV) v = _maxV;
    _value = v;
}

void RotaryEncoder::setBoundaries(int minV, int maxV, bool wrap) {
    // Ensure min <= max.
    if (minV > maxV) {
        int tmp = minV;
        minV = maxV;
        maxV = tmp;
    }
    _minV = minV;
    _maxV = maxV;
    _wrap = wrap;
    if (_value < _minV) _value = _minV;
    if (_value > _maxV) _value = _maxV;
}

void RotaryEncoder::attachListener(IEncoderListener* l) {
    if (!l) return;
    for (int i = 0; i < _listenerCount; ++i) if (_listeners[i] == l) return;
    if (_listenerCount < MAX_LISTENERS) _listeners[_listenerCount++] = l;
}

void RotaryEncoder::detachListener(IEncoderListener* l) {
    if (!l) return;
    for (int i = 0; i < _listenerCount; ++i) {
        if (_listeners[i] == l) {
            for (int j = i; j < _listenerCount - 1; ++j) _listeners[j] = _listeners[j + 1];
            _listeners[--_listenerCount] = nullptr;
            return;
        }
    }
}

void RotaryEncoder::notify(Event ev, int value) {
    // Safe iteration: check for nullptr on each call (listener may detach during callback).
    for (int i = 0; i < _listenerCount; ++i) {
        if (_listeners[i]) _listeners[i]->onEvent(ev, value);
    }
}

// Main polling logic: call frequently from the loop.
void RotaryEncoder::update() {
    // Read inputs once.
    int clk = digitalRead(_clkPin);
    int dt  = digitalRead(_dtPin);
    int sw  = digitalRead(_swPin);
    unsigned long now = millis();

    // Rotation: use transition table (quadrature state machine).
    uint8_t cur = (clk << 1) | dt;
    if (cur != _lastState) {
        uint8_t idx = (_lastState << 2) | cur;
        int8_t delta = TRANS_TABLE[idx];

        // If delta == 0: skip (noise or multi-state jump).
        // Restoring via state->position mapping may lose steps — avoid that.
        if (delta != 0) {
            _accum += delta;
            // Protect accumulator from overflow: clamp to a safe range.
            if (_accum > 127) _accum = 127;  // ~31 full steps — safe limit.
            else if (_accum < -127) _accum = -127;

            // Treat half-step encoders as one full step per click.
            int fullSteps = _accum / 2;
            // Limit step burst to a maximum per update to avoid large jumps.
            if (fullSteps > (int)MAX_FULL_STEPS_PER_UPDATE) fullSteps = MAX_FULL_STEPS_PER_UPDATE;
            else if (fullSteps < -(int)MAX_FULL_STEPS_PER_UPDATE) fullSteps = -((int)MAX_FULL_STEPS_PER_UPDATE);

            if (fullSteps != 0) {
                const int stepsAbs = abs(fullSteps);

                // dt using millis() subtraction is wrap-safe (unsigned subtraction).
                unsigned long dt_ms = (_lastStepMillis == 0) ? 1UL : (unsigned long)(now - _lastStepMillis);
                // Note: use minimum 1UL for the first step to avoid zero velocity.

                // Improved velocity filtering: compute per-step instead of batch.
                float inst_vel = 0.0f;
                if (dt_ms > 0 && stepsAbs > 0) {
                    // Compute average time per step.
                    float perStepMs = (float)dt_ms / (float)stepsAbs;
                    if (perStepMs > 0.1f) {
                        inst_vel = 1000.0f / perStepMs;  // steps per second.
                        if (inst_vel > 0.0f && inst_vel < 1000.0f) {  // reasonable bounds.
                            // Use higher weight (0.6) for faster response.
                            _vel = 0.6f * inst_vel + 0.4f * _vel;
                        }
                    }
                }

                // Acceleration based on instantaneous velocity rather than batch time.
                int mult = 1;
                if (_accelEnabled && inst_vel > 0.0f) {
                    // Acceleration thresholds are expressed in steps/sec.
                    float fastThresholdVel = 1000.0f / (float)_accelFastMs;
                    float medThresholdVel = 1000.0f / (float)_accelMedMs;

                    if (inst_vel >= fastThresholdVel) mult = _accelFastMult;
                    else if (inst_vel >= medThresholdVel) mult = _accelMedMult;
                }

                int32_t old = _value;
                // Apply all full steps at once (preserve sign). Perform calculations in 32-bit.
                int32_t deltaValue = (int32_t)fullSteps * (int32_t)_steps * (int32_t)mult;
                int32_t newValue = (int32_t)_value + deltaValue;

                if (_wrap) {
                    int32_t range = (int32_t)(_maxV) - (int32_t)(_minV) + 1;
                    if (range > 1) {  // Apply wrapping only when the range contains more than 1 value.
                        int32_t offset = ((int32_t)newValue - (int32_t)_minV) % range;
                        if (offset < 0) offset += range;
                        newValue = (int32_t)_minV + offset;
                    } else if (range == 1) {
                        // Single-value range: always stay at min (equals max).
                        newValue = _minV;
                    }
                    // If range <= 0, ignore (invalid configuration; do not apply wrap).
                } else {
                    if (newValue > (int32_t)_maxV) newValue = _maxV;
                    if (newValue < (int32_t)_minV) newValue = _minV;
                }

                _value = (int32_t)newValue;
                if (_value != old) {
                    notify((fullSteps > 0) ? INCREMENT : DECREMENT, (int)_value);
                }

                _accum -= fullSteps * 2; // Leave the remainder (-1..1).
                _lastStepMillis = now;
            }
        }

        _lastState = cur;
    }

    // Button debounce (keep raw and debounced states separate).
    if (sw != _lastRawSw) {
        _lastRawSw = sw;
        _lastSwMillis = now;
    } else if (sw != _swState && (now - _lastSwMillis) > ROTARY_BTN_DEBOUNCE_MS) {
        _swState = sw;
        if (!_btnDown && _swState == LOW) {
            _btnDown = true;
            notify(PRESS_START, 0);
        } else if (_btnDown && _swState == HIGH) {
            _btnDown = false;
            notify(PRESS_END, 0);
        }
    }
}
