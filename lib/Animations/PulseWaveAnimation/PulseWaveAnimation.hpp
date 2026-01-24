#ifndef PULSE_WAVE_ANIMATION_HPP
#define PULSE_WAVE_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef PULSEWAVE_DEFAULT_HUE
#define PULSEWAVE_DEFAULT_HUE 200  // cyan
#endif
#ifndef PULSEWAVE_DEFAULT_SAT
#define PULSEWAVE_DEFAULT_SAT 255
#endif
#ifndef PULSEWAVE_DEFAULT_VAL
#define PULSEWAVE_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class PulseWaveAnimation : public AnimationBase {
public:
	explicit PulseWaveAnimation(LedMatrix& m);
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
	void render() override;

private:
	uint8_t pulseRadius;  // 0..255
};

#endif // PULSE_WAVE_ANIMATION_HPP
