#include <Arduino.h>
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
#include "../lib/AppController/AppController.hpp"

// глобальные компоненты
LedMatrix matrix;
RotaryEncoder rotary;
StarsAnimation stars(matrix);
RainbowChaseAnimation rainbow(matrix);
PlasmaAnimation plasma(matrix);
SparkleWaveAnimation sparkleWave(matrix);
PulseWaveAnimation pulseWave(matrix);
SegmentRunnerAnimation segmentRunner(matrix);
CenterPulseAnimation centerPulse(matrix);
MatrixCodeRainAnimation codeRain(matrix);
EqualizerBarsAnimation equalizerBars(matrix);
EnergyCirclesAnimation energyCircles(matrix);
ReactorTurbinesAnimation reactorTurbines(matrix);
ChargingPulseAnimation chargingPulse(matrix);
AppController app(matrix, rotary);

void setup() {
	#if DEBUG_SERIAL
	Serial.begin(115200);
	while (!Serial) { delay(10); }
	Serial.println("\n\n========================================");
	Serial.println("  LedLine - LED Matrix Animation System");
	Serial.println("========================================");
	Serial.println("Starting LedLine...");
	#endif

	// init NVS (Preferences)
	{
		Preferences prefs;
		prefs.begin("app", false);
		prefs.end();
	}

	// init hardware
	matrix.init();

	// set readable animation names
	stars.setName("Stars");
	rainbow.setName("Rainbow Chase");
	segmentRunner.setName("Segment Runner");
	centerPulse.setName("Center Pulse");
	plasma.setName("Plasma");
	sparkleWave.setName("Sparkle Wave");
	pulseWave.setName("Pulse Wave");
	codeRain.setName("Matrix Code Rain");
	equalizerBars.setName("Equalizer Bars");
	energyCircles.setName("Energy Circles");
	reactorTurbines.setName("Reactor Turbines");
	chargingPulse.setName("Charging Pulse");

	// register animations
	app.addAnimation(&centerPulse);
	app.addAnimation(&segmentRunner);
	app.addAnimation(&codeRain);
	app.addAnimation(&stars);
	app.addAnimation(&sparkleWave);
	app.addAnimation(&pulseWave);
	app.addAnimation(&energyCircles);
	app.addAnimation(&reactorTurbines);
	app.addAnimation(&plasma);
	app.addAnimation(&equalizerBars);
	app.addAnimation(&chargingPulse);
	app.addAnimation(&rainbow);
	
	
	

	// start controller (attaches to rotary)
	app.begin();

	#if DEBUG_SERIAL
	Serial.println("Setup complete. Debug mode ENABLED.");
	Serial.println("Ready for hardware testing!");
	Serial.println("========================================\n");
	#endif
}

void loop() {
	// poll encoder (produces events to AppController)
	rotary.update();

	// update app (renders animations / handles poweroff)
	app.update();
}
