#pragma once
#include <Arduino.h>

// Base class for overlay/system animations rendered 
class OverlayAnimation {
public:
    virtual void render() = 0;

    // Optional hooks for overlays that support progress 
    virtual void setProgress(uint8_t) {}

};
