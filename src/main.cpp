#include <Arduino.h>
#include "debug.hpp"
#include <Preferences.h>
#include "../lib/LedMatrix/LedMatrix.hpp"
#include "../lib/RotaryEncoder/RotaryEncoder.hpp"
#include "../lib/Animations/StarsAnimation/StarsAnimation.hpp"
#include "../lib/Animations/RainbowChaseAnimation/RainbowChaseAnimation.hpp"
#include "../lib/Animations/PlasmaAnimation/PlasmaAnimation.hpp"
#include "../lib/Animations/SparkleWaveAnimation/SparkleWaveAnimation.hpp"
#include "../lib/Animations/PulseWaveAnimation/PulseWaveAnimation.hpp"
#include "../lib/Animations/SegmentRunnerAnimation/SegmentRunnerAnimation.hpp"
#include "../lib/Animations/CenterPulseAnimation/CenterPulseAnimation.hpp"
#include "../lib/Animations/MatrixCodeRainAnimation/MatrixCodeRainAnimation.hpp"
#include "../lib/Animations/EqualizerBarsAnimation/EqualizerBarsAnimation.hpp"
#include "../lib/Animations/EnergyCirclesAnimation/EnergyCirclesAnimation.hpp"
#include "../lib/Animations/ReactorTurbinesAnimation/ReactorTurbinesAnimation.hpp"
#include "../lib/Animations/ChargingPulseAnimation/ChargingPulseAnimation.hpp"
#include "../lib/Animations/PowerOffAnimation/PowerOffAnimation.hpp"
#include "../lib/AnimationManager/AnimationManager.hpp"
#include "../lib/StorageManager/StorageManager.hpp"
#include "../lib/AppManager/AppManager.hpp"

// Global components
LedMatrix matrix;
RotaryEncoder rotary;
StorageManager storage;
AnimationManager animMgr(storage);
AppManager app(animMgr, rotary, matrix, storage);


#define ANIM_ID_STARS 1
#define ANIM_ID_RAINBOW 2
#define ANIM_ID_PLASMA 3
#define ANIM_ID_SPARKLE_WAVE 4
#define ANIM_ID_PULSE_WAVE 5
#define ANIM_ID_SEGMENT_RUNNER 6
#define ANIM_ID_CENTER_PULSE 7
#define ANIM_ID_CODE_RAIN 8
#define ANIM_ID_EQUALIZER_BARS 9
#define ANIM_ID_ENERGY_CIRCLES 10
#define ANIM_ID_REACTOR_TURBINES 11
#define ANIM_ID_CHARGING_PULSE 12



StarsAnimation stars(ANIM_ID_STARS, matrix);
RainbowChaseAnimation rainbow(ANIM_ID_RAINBOW, matrix);
PlasmaAnimation plasma(ANIM_ID_PLASMA, matrix);
SparkleWaveAnimation sparkleWave(ANIM_ID_SPARKLE_WAVE, matrix);
PulseWaveAnimation pulseWave(ANIM_ID_PULSE_WAVE, matrix);
SegmentRunnerAnimation segmentRunner(ANIM_ID_SEGMENT_RUNNER, matrix);
CenterPulseAnimation centerPulse(ANIM_ID_CENTER_PULSE, matrix);
MatrixCodeRainAnimation codeRain(ANIM_ID_CODE_RAIN, matrix);
EqualizerBarsAnimation equalizerBars(ANIM_ID_EQUALIZER_BARS, matrix);
EnergyCirclesAnimation energyCircles(ANIM_ID_ENERGY_CIRCLES, matrix);
ReactorTurbinesAnimation reactorTurbines(ANIM_ID_REACTOR_TURBINES, matrix);
ChargingPulseAnimation chargingPulse(ANIM_ID_CHARGING_PULSE, matrix);



void setup() {
	#if LOG_ENABLED
	Serial.begin(115200);
	// Do not block waiting for a host serial connection — allow the device to run
	// even when no serial console is attached.
	// If you need to wait for a USB CDC connection on native USB boards,
	// enable an explicit build-time flag and implement a conditional wait.
	LOG_PRINTLN("\n\n========================================");
	LOG_PRINTLN("  LumioMonolith - LED Matrix Animation System");
	LOG_PRINTLN("========================================");
	LOG_PRINTLN("Starting LumioMonolith...");
	#endif

	matrix.init();
	rotary.init();
		// Register animations with the manager
	animMgr.addAnimation(&centerPulse);
	animMgr.addAnimation(&pulseWave);
	animMgr.addAnimation(&segmentRunner);
	animMgr.addAnimation(&energyCircles);
	animMgr.addAnimation(&stars);
	animMgr.addAnimation(&codeRain);
	animMgr.addAnimation(&equalizerBars);
	animMgr.addAnimation(&plasma);
	animMgr.addAnimation(&sparkleWave);
	animMgr.addAnimation(&reactorTurbines);
	animMgr.addAnimation(&chargingPulse);
	animMgr.addAnimation(&rainbow);

	animMgr.init();
	rotary.attachListener(&app);
	
	app.begin();

	#if LOG_ENABLED
	LOG_PRINTLN("Setup complete. Debug mode ENABLED.");
	LOG_PRINTLN("Ready for hardware testing!");
	LOG_PRINTLN("========================================\n");
	#endif
}

void loop() {
	// update app (polls rotary, renders animations / handles poweroff)
	app.update();
}
