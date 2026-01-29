#include <Arduino.h>
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

// глобальные компоненты
LedMatrix matrix;
RotaryEncoder rotary;
StorageManager storage;
AnimationManager animMgr(matrix);

// Fixed animation IDs (replaces nextAnimId)
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

// Auto-generate stable unique IDs in declaration order
// static uint16_t nextAnimId() { static uint16_t id = 1; return id++; }

StarsAnimation stars(ANIM_ID_STARS);
RainbowChaseAnimation rainbow(ANIM_ID_RAINBOW);
PlasmaAnimation plasma(ANIM_ID_PLASMA);
SparkleWaveAnimation sparkleWave(ANIM_ID_SPARKLE_WAVE);
PulseWaveAnimation pulseWave(ANIM_ID_PULSE_WAVE);
SegmentRunnerAnimation segmentRunner(ANIM_ID_SEGMENT_RUNNER);
CenterPulseAnimation centerPulse(ANIM_ID_CENTER_PULSE);
MatrixCodeRainAnimation codeRain(ANIM_ID_CODE_RAIN);
EqualizerBarsAnimation equalizerBars(ANIM_ID_EQUALIZER_BARS);
EnergyCirclesAnimation energyCircles(ANIM_ID_ENERGY_CIRCLES);
ReactorTurbinesAnimation reactorTurbines(ANIM_ID_REACTOR_TURBINES);
ChargingPulseAnimation chargingPulse(ANIM_ID_CHARGING_PULSE);
AppManager app(animMgr, rotary, matrix, storage);

void setup() {
	#if DEBUG_SERIAL
	Serial.begin(115200);
	while (!Serial) { delay(10); }
	Serial.println("\n\n========================================");
	Serial.println("  LumioMonolith - LED Matrix Animation System");
	Serial.println("========================================");
	Serial.println("Starting LumioMonolith...");
	#endif

	matrix.init();
	rotary.init();
	rotary.attachListener(&app);
	
	// register animations directly via AnimationManager
	animMgr.addAnimation(&centerPulse);
	animMgr.addAnimation(&pulseWave);
	animMgr.addAnimation(&segmentRunner);
	animMgr.addAnimation(&energyCircles);
	animMgr.addAnimation(&stars);
	animMgr.addAnimation(&codeRain);
	animMgr.addAnimation(&equalizerBars);
	animMgr.addAnimation(&plasma);
	animMgr.addAnimation(&reactorTurbines);
	animMgr.addAnimation(&chargingPulse);
	animMgr.addAnimation(&rainbow);
	
	app.begin();

	#if DEBUG_SERIAL
	Serial.println("Setup complete. Debug mode ENABLED.");
	Serial.println("Ready for hardware testing!");
	Serial.println("========================================\n");
	#endif
}

void loop() {
	// update app (polls rotary, renders animations / handles poweroff)
	app.update();
}
