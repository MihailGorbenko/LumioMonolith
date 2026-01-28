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
#include "../lib/AppController/AppController.hpp"

// глобальные компоненты
LedMatrix matrix;
RotaryEncoder rotary;

// Auto-generate stable unique IDs in declaration order
static uint16_t nextAnimId() { static uint16_t id = 1; return id++; }

StarsAnimation stars(nextAnimId());
RainbowChaseAnimation rainbow(nextAnimId());
PlasmaAnimation plasma(nextAnimId());
SparkleWaveAnimation sparkleWave(nextAnimId());
PulseWaveAnimation pulseWave(nextAnimId());
SegmentRunnerAnimation segmentRunner(nextAnimId());
CenterPulseAnimation centerPulse(nextAnimId());
MatrixCodeRainAnimation codeRain(nextAnimId());
EqualizerBarsAnimation equalizerBars(nextAnimId());
EnergyCirclesAnimation energyCircles(nextAnimId());
ReactorTurbinesAnimation reactorTurbines(nextAnimId());
ChargingPulseAnimation chargingPulse(nextAnimId());
AppController app(matrix);

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
	// configure rotary encoder directly
	rotary.attachListener(&app);
	rotary.init();
	
	// register animations
	app.addAnimation(&centerPulse);
	app.addAnimation(&pulseWave);
	app.addAnimation(&segmentRunner);
	app.addAnimation(&energyCircles);
	app.addAnimation(&stars);
	app.addAnimation(&codeRain);
	app.addAnimation(&equalizerBars);
	app.addAnimation(&plasma);
	app.addAnimation(&reactorTurbines);
	app.addAnimation(&chargingPulse);
	app.addAnimation(&rainbow);
	
	app.begin();

	#if DEBUG_SERIAL
	Serial.println("Setup complete. Debug mode ENABLED.");
	Serial.println("Ready for hardware testing!");
	Serial.println("========================================\n");
	#endif
}

void loop() {
	// poll rotary encoder (produces events to AppController)
	rotary.update();
	// update app (renders animations / handles poweroff)
	app.update();
}
