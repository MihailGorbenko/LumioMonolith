#pragma once
#include <Arduino.h>

// Base class for overlay/system animations rendered without passing LedMatrix in render()
class OverlayAnimation {
public:
    virtual ~OverlayAnimation() {}
    virtual void render() = 0;

    // Optional hooks for overlays that support progress or color
    virtual void setProgress(uint8_t) {}

};
