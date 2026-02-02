#ifndef PULSE_WAVE_ANIMATION_HPP
#define PULSE_WAVE_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef PULSEWAVE_DEFAULT_HUE
#define PULSEWAVE_DEFAULT_HUE 200  // cyan
#endif
#ifndef PULSEWAVE_DEFAULT_SAT
#define PULSEWAVE_DEFAULT_SAT 255
#endif
#ifndef PULSEWAVE_DEFAULT_VAL
#define PULSEWAVE_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Readable name
#ifndef PULSEWAVE_ANIMATION_NAME
#define PULSEWAVE_ANIMATION_NAME "Pulse Wave"
#endif

class PulseWaveAnimation : public AnimationBase {
public:
	explicit PulseWaveAnimation(uint16_t id, LedMatrix* m);
	void render() override;
	const char* getName() const override { return PULSEWAVE_ANIMATION_NAME; }

private:
	uint8_t pulseRadius;  // 0..255
};


#endif // PULSE_WAVE_ANIMATION_HPP
