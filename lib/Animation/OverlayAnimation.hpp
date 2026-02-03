#pragma once
#include <Arduino.h>
#include "../LedMatrix/LedMatrix.hpp"

// Base class for overlay/system animations rendered
class OverlayAnimation {
public:
    explicit OverlayAnimation(LedMatrix& m, uint8_t h = 0, uint8_t s = 255, uint8_t v = 255)
        : matrix(m), hue(h), sat(s), val(v), progress(0) {}
    virtual ~OverlayAnimation() = default;

    virtual void render() = 0;

    // Optional hooks for overlays that support progress
    virtual void setProgress(uint8_t p) { progress = p; }

protected:
    LedMatrix& matrix;
    uint8_t hue;
    uint8_t sat;
    uint8_t val;
    uint8_t progress;
};

