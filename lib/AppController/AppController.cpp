#include "AppController.hpp"

#if DEBUG_SERIAL
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#define DBG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DBG_PRINT(x)
#define DBG_PRINTLN(x)
#define DBG_PRINTF(...)
#endif

AppController::AppController(LedMatrix& m, RotaryEncoder& enc)
	: matrix(&m),
	  encoder(&enc),
	  currentIndex(0),
	  mode(MODE_BRIGHTNESS),
	  powered(true),
	  powered_off_shown(false),
	  brightStep(APP_STEPS/2),
	  colorStep(0),
	  btnDown(false),
	  btnPressedMillis(0),
	  powerOffAnim(m),
      powerOnAnim(m),
	  lastFrameMillis(0),
	  frameIntervalMs(0),
	  lastActivityMillis(0),
      powerOnStartMillis(0),
      powerOnUntilMs(0) {
	// clamp to avoid 0ms interval on misconfigured APP_FPS
	unsigned long interval = (APP_FPS > 0) ? (1000UL / (unsigned long)APP_FPS) : 33UL;
	if (interval == 0) interval = 1;
	frameIntervalMs = interval;
}

void AppController::addAnimation(AnimationBase* a) {
	if (!a) return;
	animations.push_back(a);
}

void AppController::begin() {
	DBG_PRINTLN("[AppController] Initializing...");
	encoder->attachListener(this);
	encoder->init();

	// configure accel from controller (x3 on fast)
	encoder->setAccelThresholds(100, 40);
	encoder->setAccelMultipliers(2, 3);
	encoder->setAccelEnabled(true);

	loadState();
	applyMasterBrightness();
	if (!animations.empty() && currentIndex >= 0 && currentIndex < (int)animations.size()) {
		DBG_PRINTF("[AppController] Loaded animation index: %d (%s), brightness: %d\n", currentIndex, animations[currentIndex]->getName(), brightStep);
	} else {
		DBG_PRINTF("[AppController] Loaded animation index: %d, brightness: %d\n", currentIndex, brightStep);
	}

	// sync encoder to current mode/value so acceleration affects controller logic
	switch (mode) {
		case MODE_SELECT_ANIM:
			if (!animations.empty()) {
				encoder->setBoundaries(0, (int)animations.size() - 1, true);
				encoder->setValue(currentIndex);
			} else {
				encoder->setBoundaries(0, 0, false);
				encoder->setValue(0);
			}
			break;
		case MODE_BRIGHTNESS:
			encoder->setBoundaries(0, APP_STEPS - 1, false);
			encoder->setValue(brightStep);
			break;
		case MODE_COLOR:
			encoder->setBoundaries(0, APP_COLOR_STEPS - 1, false);
			encoder->setValue(colorStep);
			break;
		case MODE_POWEROFF:
			break;
	}

	if (!animations.empty()) {
		if (currentIndex < 0 || currentIndex >= (int)animations.size()) currentIndex = 0;
		// try load animation-specific settings (optional)
		char key[32];
		snprintf(key, sizeof(key), "anim_%d", currentIndex);
		animations[currentIndex]->loadFromNVS(key);
		// notify animation that it's now active (for warmups, etc.)
		animations[currentIndex]->onActivate();
	}
	DBG_PRINTLN("[AppController] Initialization complete");
	lastActivityMillis = millis();
}

void AppController::update() {
	unsigned long now = millis();

	if (btnDown && powered) {
		unsigned long held = now - btnPressedMillis;
		if (held >= APP_POWEROFF_HOLD_MS) {
			if (!matrix) return;  // safety check
			powerOffAnim.setProgress(0);

			// show final frame, then power off
			matrix->clear();
			powerOffAnim.render();
			matrix->show();

			// save current animation color before powering off
			if (!animations.empty() && currentIndex >= 0 && currentIndex < (int)animations.size()) {
				char key[32];
				snprintf(key, sizeof(key), "anim_%d", currentIndex);
				animations[currentIndex]->saveToNVS(key);
			}
			powered = false;
			mode = MODE_POWEROFF;
			saveState();		DBG_PRINTLN("[AppController] POWERED OFF - state saved to NVS");			// clear matrix completely after power-off sequence
			matrix->clear();
			matrix->show();
		} else {
			// Show full-lit frame immediately, then extinguish after minimal threshold
			int prog;
			if (held >= APP_POWEROFF_MIN_ANIM_MS) {
				prog = 255 - (int)((uint32_t)held * 255 / APP_POWEROFF_HOLD_MS);
				if (prog < 0) prog = 0;
			} else {
				prog = 255; // all segments ON while initial hold
			}
			powerOffAnim.setProgress((uint8_t)prog);

			// ensure it's visible while holding
			matrix->clear();
			powerOffAnim.render();
			matrix->show();
		}
		return; // while holding, ignore other rendering
	}

	if (!powered) {
		// keep matrix cleared while powered off (only once, not every cycle)
		if (!powered_off_shown && matrix) {
			matrix->clear();
			matrix->show();
			powered_off_shown = true;
		}
		return;
	}

	// On power-on, show the plasma shot animation briefly
	if ((long)(now - powerOnUntilMs) < 0) {
		unsigned long elapsed = now - powerOnStartMillis;
		if (elapsed > (unsigned long)APP_POWERON_ANIM_MS) elapsed = (unsigned long)APP_POWERON_ANIM_MS;
		uint8_t prog = (uint8_t)((elapsed * 255UL) / (unsigned long)APP_POWERON_ANIM_MS);
		powerOnAnim.setProgress(prog);
		powerOnAnim.render();
		return; // while power-on animation plays, skip normal rendering
	}

	// Auto-switch to brightness on inactivity
	if ((mode == MODE_SELECT_ANIM || mode == MODE_COLOR) && (millis() - lastActivityMillis >= APP_IDLE_TIMEOUT_MS)) {
		// if leaving color mode, save the selected color for current animation
		if (mode == MODE_COLOR) {
			if (!animations.empty() && currentIndex >= 0 && currentIndex < (int)animations.size()) {
				char key[32];
				snprintf(key, sizeof(key), "anim_%d", currentIndex);
				animations[currentIndex]->saveToNVS(key);
			}
		}
		mode = MODE_BRIGHTNESS;
		DBG_PRINTLN("[AppController] Auto-switch to BRIGHTNESS due to inactivity");
		if (encoder) {
			encoder->setBoundaries(0, APP_STEPS - 1, false);
			encoder->setValue(brightStep);
		}
		lastActivityMillis = millis();
	}

	if (animations.empty()) return;
	// Safe frame timing: protect against millis() wrap (~49 days on 32-bit)
	unsigned long elapsed = (unsigned long)(now - lastFrameMillis);
	if (elapsed < frameIntervalMs) return;
	lastFrameMillis = now;

	// In selection mode, render the selected animation directly (no progress overlay)
	if (mode == MODE_SELECT_ANIM) {
		if (currentIndex >= 0 && currentIndex < (int)animations.size()) {
			animations[currentIndex]->render();
		}
	} else {
		// bounds-checked animation render
		if (currentIndex >= 0 && currentIndex < (int)animations.size()) {
			animations[currentIndex]->render();
		}
	}
}

void AppController::onEvent(RotaryEncoder::Event ev, int value) {
	// register user interaction for idle timeout
	lastActivityMillis = millis();
	if (ev == RotaryEncoder::PRESS_START) {
		btnDown = true;
		btnPressedMillis = millis();
		return;
	}

	if (ev == RotaryEncoder::PRESS_END) {
		unsigned long held = millis() - btnPressedMillis;
		btnDown = false;

		if (!powered) {
			if (held < APP_POWEROFF_HOLD_MS) {
				powered = true;
				powered_off_shown = false;  // reset flag for next power-off
				loadState();
				if (matrix) {
					// clear screen before restoring animation
					matrix->clear();
					matrix->show();
				}
				applyMasterBrightness();
				mode = MODE_BRIGHTNESS;
				DBG_PRINTLN("[AppController] POWERED ON - state restored from NVS");

				// restore current animation settings (if any)
				if (!animations.empty() && currentIndex >= 0 && currentIndex < (int)animations.size()) {
					char key[32];
					snprintf(key, sizeof(key), "anim_%d", currentIndex);
					animations[currentIndex]->loadFromNVS(key);
					// notify animation that it's now active (for warmups, etc.)
					animations[currentIndex]->onActivate();
				}

				// sync encoder after power-on according to current mode
				switch (mode) {
					case MODE_SELECT_ANIM:
						if (!animations.empty()) {
							encoder->setBoundaries(0, (int)animations.size() - 1, true);
							encoder->setValue(currentIndex);
						} else {
							encoder->setBoundaries(0, 0, false);
							encoder->setValue(0);
						}
						break;
					case MODE_BRIGHTNESS:
						encoder->setBoundaries(0, APP_STEPS - 1, false);
						encoder->setValue(brightStep);
						break;
					case MODE_COLOR:
						encoder->setBoundaries(0, APP_STEPS - 1, false);
						encoder->setValue(colorStep);
						break;
					case MODE_POWEROFF:
						break;
				}

				// trigger power-on animation
				powerOnStartMillis = millis();
				powerOnUntilMs = powerOnStartMillis + (unsigned long)APP_POWERON_ANIM_MS;
			}
			return;
		}

		if (held >= APP_POWEROFF_HOLD_MS) return;

		// short press: cycle modes (Brightness -> Select Animation -> Color)
		if (mode == MODE_COLOR) {
			if (!animations.empty() && currentIndex >= 0 && currentIndex < (int)animations.size()) {
				char key[32];
				snprintf(key, sizeof(key), "anim_%d", currentIndex);
				animations[currentIndex]->saveToNVS(key);
			}
			mode = MODE_BRIGHTNESS;
			DBG_PRINTLN("[AppController] Mode: BRIGHTNESS (color saved)");
		} else if (mode == MODE_BRIGHTNESS) {
			mode = MODE_SELECT_ANIM;
			DBG_PRINTLN("[AppController] Mode: SELECT_ANIM");
		} else if (mode == MODE_SELECT_ANIM) {
			mode = MODE_COLOR;
			DBG_PRINTLN("[AppController] Mode: COLOR");
		} else {
			mode = MODE_BRIGHTNESS;
			DBG_PRINTLN("[AppController] Mode: BRIGHTNESS");
		}

		// sync encoder to new mode/value (so accel affects correct parameter)
		switch (mode) {
			case MODE_SELECT_ANIM:
				if (!animations.empty()) {
					encoder->setBoundaries(0, (int)animations.size() - 1, true);
					encoder->setValue(currentIndex);
				}
				break;
			case MODE_BRIGHTNESS:
				encoder->setBoundaries(0, APP_STEPS - 1, false);
				encoder->setValue(brightStep);
				break;
			case MODE_COLOR:
				encoder->setBoundaries(0, APP_COLOR_STEPS - 1, false);
				encoder->setValue(colorStep);
				break;
			case MODE_POWEROFF:
				break;
		}
		return;
	}

	// rotation: use encoder 'value' (acceleration now works)
	if (ev == RotaryEncoder::INCREMENT || ev == RotaryEncoder::DECREMENT) {
		if (!powered || btnDown) return;

		switch (mode) {
			case MODE_SELECT_ANIM:
				if (animations.empty() || !matrix || !encoder) return;
				{
					int newIndex = constrain(value, 0, (int)animations.size() - 1);
					if (newIndex == currentIndex) return;

					matrix->clear();
					matrix->show();

					currentIndex = newIndex;
					DBG_PRINTF("[AppController] Animation changed to: %d (%s)\n", currentIndex, animations[currentIndex]->getName());
					char key[32];
					snprintf(key, sizeof(key), "anim_%d", currentIndex);
					animations[currentIndex]->loadFromNVS(key);
					// notify animation that it's now active (for warmups, etc.)
					animations[currentIndex]->onActivate();
					// No progress overlay; directly preview current animation
				}
				break;

			case MODE_BRIGHTNESS:
				brightStep = constrain(value, 0, APP_STEPS - 1);
				applyMasterBrightness();
				saveState();
				DBG_PRINTF("[AppController] Brightness: %d/%d\n", brightStep, APP_STEPS);
				break;

			case MODE_COLOR:
				colorStep = constrain(value, 0, APP_COLOR_STEPS - 1);
				if (!matrix || animations.empty()) break;
				if (currentIndex < 0 || currentIndex >= (int)animations.size()) break;
				{
					int delta = (colorStep * 256) / APP_COLOR_STEPS;
					if (delta > 255) delta = 255;  // clamp to valid hue range
					animations[currentIndex]->setColorHSV((uint8_t)delta, 255, 255);
					DBG_PRINTF("[AppController] Color (Hue): %d (%d/%d)\n", delta, colorStep, APP_COLOR_STEPS);
				}
				break;

			case MODE_POWEROFF:
				break;
		}
	}
}

bool AppController::saveState() {
	Preferences prefs;
	if (!prefs.begin("app", false)) {
		DBG_PRINTLN("[NVS] Error: Failed to begin NVS write");
		return false;
	}
	prefs.putUInt("lastAnim", (uint32_t)currentIndex);
	prefs.putUShort("brightStep", (uint16_t)brightStep);
	prefs.end();
	DBG_PRINTF("[NVS] State saved: anim=%d, brightness=%d\n", currentIndex, brightStep);
	return true;
}

bool AppController::loadState() {
	Preferences prefs;
	if (!prefs.begin("app", true)) {
		currentIndex = 0;
		brightStep = APP_STEPS/2;
		DBG_PRINTLN("[NVS] Warning: Could not read state (using defaults)");
		return false;
	}
	uint32_t ai = prefs.getUInt("lastAnim", 0);
	uint16_t bs = prefs.getUShort("brightStep", (uint16_t)(APP_STEPS/2));
	prefs.end();
	currentIndex = (int)ai;
	brightStep = (int)bs;
	if (currentIndex < 0) currentIndex = 0;
	if (brightStep < 0) brightStep = 0;
	if (brightStep >= APP_STEPS) brightStep = APP_STEPS - 1;
	DBG_PRINTF("[NVS] State loaded: anim=%d, brightness=%d\n", currentIndex, brightStep);
	return true;
}

void AppController::applyMasterBrightness() {
	int bs = constrain(brightStep, 0, APP_STEPS - 1);
	// Linear interpolation: ensure max brightness (255) at max step
	unsigned long v;
	if (bs == APP_STEPS - 1) {
		v = 255;
	} else {
		v = 5 + (unsigned long)bs * 250 / (APP_STEPS - 1);
	}
	if (v > 255) v = 255;  // ensure no overflow
	matrix->setMasterBrightness((uint8_t)v);
}