#ifndef FLUORO_LAMP_ANIMATION_HPP
#define FLUORO_LAMP_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

// Default: low saturation to look like white fluorescent light
#ifndef FLUOROLAMP_DEFAULT_HUE
#define FLUOROLAMP_DEFAULT_HUE 0
#endif
#ifndef FLUOROLAMP_DEFAULT_SAT
#define FLUOROLAMP_DEFAULT_SAT 10
#endif
#ifndef FLUOROLAMP_DEFAULT_VAL
#define FLUOROLAMP_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class FluoroLampAnimation : public AnimationBase {
public:
	explicit FluoroLampAnimation(LedMatrix& m);
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
	void onActivate() override { reset(); }
	void render() override;

	// Adjust warmup duration (ms) and flicker strength (0..255)
	void setWarmupMs(uint16_t ms);
	void setFlickerStrength(uint8_t amt);
	void reset();

private:
	// Timing and state
	unsigned long startedAt;
	uint16_t warmupMs;
	uint8_t flickerAmt; // how strong ongoing flicker is

	// Warmup blinking segment
	unsigned long blinkUntilMs;
	unsigned long nextBlinkAtMs;
	uint8_t blinkStartX;
	uint8_t blinkLen;

	void scheduleBlink(int w);
};

#endif // FLUORO_LAMP_ANIMATION_HPP
