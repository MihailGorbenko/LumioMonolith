#include "RotaryEncoder.hpp"
#include <stdint.h>
#include <stdlib.h> // abs(int)

// изменённый конструктор: steps по умолчанию = 1
RotaryEncoder::RotaryEncoder()
    : _clkPin(ROTARY_CLK_PIN), _dtPin(ROTARY_DT_PIN), _swPin(ROTARY_SW_PIN),
      _steps(1),
      _value(0), _minV(0), _maxV(127), _wrap(false),
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

// таблица переходов остаётся в файле
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
    _accelFastMult = (fast_mult > 0) ? fast_mult : 3; // default fast multiplier = x3
    _velFilterAlpha = filterAlpha;
}

// added: runtime control from controller
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

    // Инициализируем state-machine (per-instance)
    _lastState = (digitalRead(_clkPin) << 1) | digitalRead(_dtPin);
    _accum = 0;

    // Инициализируем debounce-состояния согласованно
    _swState = digitalRead(_swPin);
    _lastRawSw = _swState;
    _btnDown = (_swState == LOW);
    _lastSwMillis = millis();

    // ensure velocity/step timestamps are reset
    _lastStepMillis = 0;
    _vel = 0.0f;
}

void RotaryEncoder::setValue(int v) {
    if (v < _minV) v = _minV;
    if (v > _maxV) v = _maxV;
    _value = v;
}

void RotaryEncoder::setBoundaries(int minV, int maxV, bool wrap) {
    // ensure min <= max
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
    // Safe iteration: check null at each step (listener might detach during callback)
    for (int i = 0; i < _listenerCount; ++i) {
        if (_listeners[i]) _listeners[i]->onEvent(ev, value);
    }
}

// Основная поллинг-логика: вызвать часто (loop)
void RotaryEncoder::update() {
    // read inputs once
    int clk = digitalRead(_clkPin);
    int dt  = digitalRead(_dtPin);
    int sw  = digitalRead(_swPin);
    unsigned long now = millis();

    // rotation: используем таблицу переходов (quadrature state machine)
    uint8_t cur = (clk << 1) | dt;
    if (cur != _lastState) {
        uint8_t idx = (_lastState << 2) | cur;
        int8_t delta = TRANS_TABLE[idx];

        // Если delta == 0: пропускаем. Это либо шум, либо скачок на 2+ состояния
        // Recovery через STATE_TO_POS_IDX может потерять шаги, поэтому не используем
        if (delta != 0) {
            _accum += delta;
            // Protect against accumulator overflow: clamp to safe range
            if (_accum > 127) _accum = 127;  // ~31 full steps, safe boundary
            else if (_accum < -127) _accum = -127;

            int fullSteps = _accum / 4;
            // Ограничиваем, но плавнее (не режим резкий скачок)
            if (fullSteps > (int)MAX_FULL_STEPS_PER_UPDATE) fullSteps = MAX_FULL_STEPS_PER_UPDATE;
            else if (fullSteps < -(int)MAX_FULL_STEPS_PER_UPDATE) fullSteps = -((int)MAX_FULL_STEPS_PER_UPDATE);

            if (fullSteps != 0) {
                const int stepsAbs = abs(fullSteps);

                // dt across millis() wrap is safe via unsigned subtraction
                unsigned long dt_ms = (_lastStepMillis == 0) ? 1UL : (unsigned long)(now - _lastStepMillis);
                // Note: use 1UL minimum for first step to avoid zero velocity on first rotation

                // Улучшена velocity фильтрация: per-step basis вместо batch
                float inst_vel = 0.0f;
                if (dt_ms > 0 && stepsAbs > 0) {
                    // Вычисляем среднее время между шагами
                    float perStepMs = (float)dt_ms / (float)stepsAbs;
                    if (perStepMs > 0.1f) {
                        inst_vel = 1000.0f / perStepMs;  // steps/sec
                        if (inst_vel > 0.0f && inst_vel < 1000.0f) {  // sanity bounds
                            // Используем более высокий коэффициент (0.6) для быстрого отклика
                            _vel = 0.6f * inst_vel + 0.4f * _vel;
                        }
                    }
                }

                // Ускорение на основе instantaneous velocity, а не batch time
                int mult = 1;
                if (_accelEnabled && inst_vel > 0.0f) {
                    // Порог ускорения в steps/sec
                    float fastThresholdVel = 1000.0f / (float)_accelFastMs;
                    float medThresholdVel = 1000.0f / (float)_accelMedMs;
                    
                    if (inst_vel >= fastThresholdVel) mult = _accelFastMult;
                    else if (inst_vel >= medThresholdVel) mult = _accelMedMult;
                }

                int old = _value;
                // apply all full steps at once (fullSteps signed)
                int deltaValue = fullSteps * _steps * mult;
                int newValue = _value + deltaValue;

                if (_wrap) {
                    int range = (_maxV - _minV) + 1;
                    if (range > 1) {  // only apply wrap if more than 1 value in range
                        int offset = (newValue - _minV) % range;
                        if (offset < 0) offset += range;
                        newValue = _minV + offset;
                    } else if (range == 1) {
                        // Single value: always stay at min (which equals max)
                        newValue = _minV;
                    }
                    // if range <= 0, ignore (invalid config, don't apply wrap)
                } else {
                    if (newValue > _maxV) newValue = _maxV;
                    if (newValue < _minV) newValue = _minV;
                }

                _value = newValue;
                if (_value != old) {
                    notify((fullSteps > 0) ? INCREMENT : DECREMENT, _value);
                }

                _accum -= fullSteps * 4; // keep remainder (-3..3)
                _lastStepMillis = now;
            }
        }

        _lastState = cur;
    }

    // button debounce (оставляем схему “raw отдельно, debounced отдельно”)
    if (sw != _lastRawSw) {
        _lastRawSw = sw;
        _lastSwMillis = now;
    } else if (sw != _swState && (now - _lastSwMillis) > DEBOUNCE_MS) {
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