#pragma once
#include <Arduino.h>

class PowerManager {
public:
    void begin() {}
    void update(unsigned long /*now*/) {}

    bool isPowered() const { return powered; }
    void setPowered(bool on) { powered = on; }

private:
    bool powered = true;
};
