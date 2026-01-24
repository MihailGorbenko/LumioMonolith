#include <Arduino.h>
#include <Preferences.h>
#include "../lib/LedMatrix/LedMatrix.hpp"
#include "../lib/RotaryEncoder/RotaryEncoder.hpp"
#include "../lib/Animations/StarsAnimation/StarsAnimation.hpp"
#include "../lib/Animations/RainbowChaseAnimation/RainbowChaseAnimation.hpp"
#include "../lib/Animations/ScannerAnimation/ScannerAnimation.hpp"
#include "../lib/Animations/PlasmaAnimation/PlasmaAnimation.hpp"
#include "../lib/Animations/SparkleWaveAnimation/SparkleWaveAnimation.hpp"
#include "../lib/Animations/PulseWaveAnimation/PulseWaveAnimation.hpp"
#include "../lib/Animations/MatrixRainAnimation/MatrixRainAnimation.hpp"
#include "../lib/Animations/NeonGridAnimation/NeonGridAnimation.hpp"
#include "../lib/Animations/GalacticWarpAnimation/GalacticWarpAnimation.hpp"
#include "../lib/Animations/FluoroLampAnimation/FluoroLampAnimation.hpp"
#include "../lib/Animations/WhiteStaticAnimation/WhiteStaticAnimation.hpp"
#include "../lib/Animations/PowerOffAnimation/PowerOffAnimation.hpp"
#include "../lib/AppController/AppController.hpp"

// глобальные компоненты
LedMatrix matrix;
RotaryEncoder rotary;
StarsAnimation stars(matrix);
RainbowChaseAnimation rainbow(matrix);
ScannerAnimation scanner(matrix);
PlasmaAnimation plasma(matrix);
SparkleWaveAnimation sparkleWave(matrix);
PulseWaveAnimation pulseWave(matrix);
MatrixRainAnimation matrixRain(matrix);
NeonGridAnimation neonGrid(matrix);
GalacticWarpAnimation galacticWarp(matrix);
FluoroLampAnimation fluoro(matrix);
WhiteStaticAnimation whiteStatic(matrix);
AppController app(matrix, rotary);

void setup() {
	Serial.begin(115200);
	while (!Serial) { delay(10); }
	Serial.println("\n\n========================================");
	Serial.println("  LedLine - LED Matrix Animation System");
	Serial.println("========================================");
	Serial.println("Starting LedLine...");

	// init NVS (Preferences)
	{
		Preferences prefs;
		prefs.begin("app", false);
		prefs.end();
	}

	// init hardware
	matrix.init();

	// register animations
	app.addAnimation(&stars);
	app.addAnimation(&rainbow);
	app.addAnimation(&scanner);
	app.addAnimation(&plasma);
	app.addAnimation(&sparkleWave);
	app.addAnimation(&pulseWave);
	app.addAnimation(&matrixRain);
	app.addAnimation(&neonGrid);
	app.addAnimation(&galacticWarp);
	app.addAnimation(&fluoro);
	app.addAnimation(&whiteStatic);
	

	// start controller (attaches to rotary)
	app.begin();

	Serial.println("Setup complete. Debug mode ENABLED.");
	Serial.println("Ready for hardware testing!");
	Serial.println("========================================\n");
}

void loop() {
	// poll encoder (produces events to AppController)
	rotary.update();

	// update app (renders animations / handles poweroff)
	app.update();
}
