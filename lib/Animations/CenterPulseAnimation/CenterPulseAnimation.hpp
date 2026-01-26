#ifndef CENTER_PULSE_ANIMATION_HPP
#define CENTER_PULSE_ANIMATION_HPP

#include <Arduino.h>
#include "../Animation/Animation.hpp"
#include "../StorageManager/Serializable.hpp"

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

class CenterPulseAnimation : public AnimationBase, public ISerializable {
public:
    explicit CenterPulseAnimation(LedMatrix& m);

    void setSpeedDiv(uint8_t div);   // smaller = faster
    void render() override;

private:
    uint8_t speedDiv; // time divider for sin phase

    // ISerializable
public:
    size_t serializedSize() const override { return 2; }
    bool serialize(uint8_t* out, size_t maxLen) const override;
    bool deserialize(const uint8_t* data, size_t len) override;
    ISerializable* serializable() override { return this; }
};

#endif // CENTER_PULSE_ANIMATION_HPP
