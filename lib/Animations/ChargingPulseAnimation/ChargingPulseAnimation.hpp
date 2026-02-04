#ifndef CHARGING_PULSE_ANIMATION_HPP
#define CHARGING_PULSE_ANIMATION_HPP

#include <Arduino.h>
#include <vector>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef CHARGING_DEFAULT_HUE
#define CHARGING_DEFAULT_HUE 170 // blue by default
#endif
#ifndef CHARGING_DEFAULT_SAT
#define CHARGING_DEFAULT_SAT 255
#endif
#ifndef CHARGING_DEFAULT_VAL
#define CHARGING_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Time per ring ramp (ms)
#ifndef CHARGING_RING_STEP_MS
#define CHARGING_RING_STEP_MS 180
#endif
// Top flash duration (ms)
#ifndef CHARGING_FLASH_MS
#define CHARGING_FLASH_MS 160
#endif
// Global fade duration (ms)
#ifndef CHARGING_FADE_MS
#define CHARGING_FADE_MS 260
#endif

// Readable name
#ifndef CHARGINGPULSE_ANIMATION_NAME
#define CHARGINGPULSE_ANIMATION_NAME "Charging Pulse"
#endif

class ChargingPulseAnimation : public AnimationBase {
public:
    explicit ChargingPulseAnimation(uint16_t id, LedMatrix& m);
    void onActivate() override;
    void render() override;

    const char* getName() const override { return CHARGINGPULSE_ANIMATION_NAME; }
private:
    // Cached values populated in onActivate()
    int cachedWidth = 0;
    int cachedHeight = 0;
    int cachedAscendMs = 0;
    int cachedCycleMs = 0;
    float invRingStep = 0.0f; // 1 / CHARGING_RING_STEP_MS
    float invAscend = 0.0f;   // 1 / cachedAscendMs
    std::vector<int> startMs; // start time per row
};

#endif // CHARGING_PULSE_ANIMATION_HPP
