#ifndef CENTER_PULSE_ANIMATION_HPP
#define CENTER_PULSE_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef CENTERPULSE_DEFAULT_HUE
#define CENTERPULSE_DEFAULT_HUE 170 // blue
#endif
#ifndef CENTERPULSE_DEFAULT_SAT
#define CENTERPULSE_DEFAULT_SAT 255
#endif
#ifndef CENTERPULSE_DEFAULT_VAL
#define CENTERPULSE_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Readable name
#ifndef CENTERPULSE_ANIMATION_NAME
#define CENTERPULSE_ANIMATION_NAME "Center Pulse"
#endif

class CenterPulseAnimation : public AnimationBase {
public:
    explicit CenterPulseAnimation(uint16_t id, LedMatrix* m);

    void render() override;

private:
    uint8_t speedDiv; // time divider for sin phase

    const char* getName() const override { return CENTERPULSE_ANIMATION_NAME; }
};

#endif // CENTER_PULSE_ANIMATION_HPP
