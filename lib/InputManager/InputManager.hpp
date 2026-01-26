#pragma once
#include <Arduino.h>
#include "../RotaryEncoder/RotaryEncoder.hpp"
#include "../../src/AppConfig.hpp"

class InputManager : public RotaryEncoder::IEncoderListener {
public:
    explicit InputManager(RotaryEncoder& enc) : encoder(&enc) {}

    void begin() {
        if (!encoder) return;
        // Attach self as listener and init hardware
        encoder->attachListener(this);
        encoder->init();
        // Configure acceleration thresholds
        encoder->setAccelThresholds(ENCODER_ACCEL_THRESH_SLOW, ENCODER_ACCEL_THRESH_FAST);
        // Set fixed boundaries and initial value
        encoder->setBoundaries(ENC_MIN, ENC_MAX, true);
        encoder->setValue(0);
    }

    void update() { if (encoder) encoder->update(); }

    void attachListener(RotaryEncoder::IEncoderListener* l) { listener = l; }

    void onEvent(RotaryEncoder::Event ev, int value) override {
        lastEvent = ev; lastValue = value; lastEventMs = millis();
        if (listener) listener->onEvent(ev, value);
    }

    RotaryEncoder::Event lastEvent = RotaryEncoder::NONE;
    int lastValue = 0;
    unsigned long lastEventMs = 0;

private:
    RotaryEncoder* encoder;
    RotaryEncoder::IEncoderListener* listener = nullptr;
};
