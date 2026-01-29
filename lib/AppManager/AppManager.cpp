#include "AppManager.hpp"

#if DEBUG_SERIAL
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#define DBG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DBG_PRINT(x)
#define DBG_PRINTLN(x)
#define DBG_PRINTF(...)
#endif

// Производные величины для расчёта дельты
static const int ENC_RANGE = (ENC_MAX - ENC_MIN + 1);
static const int ENC_HALF = (ENC_RANGE / 2);

AppManager::AppManager(LedMatrix& m)
	: matrix(&m),
			currentIndex(0),
			mode(MODE_BRIGHTNESS),
			appState(STATE_RUNNING),
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

void AppManager::addAnimation(AnimationBase* a) {
	if (!a) return;
	animations.push_back(a);
	animDirty.push_back(0);
}

void AppManager::begin() {
	DBG_PRINTLN("[AppManager] Initializing...");
	// Encoder base value
	encBaseValue = 0;

	// Построим gamma LUT размером APP_STEPS
	gammaLUT.resize(APP_STEPS);
	for (int i = 0; i < APP_STEPS; ++i) {
		float norm = (APP_STEPS > 1) ? ((float)i / (float)(APP_STEPS - 1)) : 1.0f;
		float g = powf(norm, APP_GAMMA) * 255.0f;
		int v = (int)lroundf(g);
		if (v < 0) v = 0; if (v > 255) v = 255;
		gammaLUT[i] = (uint8_t)v;
	}

	loadState();
	applyMasterBrightness();
	if (!animations.empty() && currentIndex >= 0 && currentIndex < (int)animations.size()) {
		DBG_PRINTF("[AppManager] Loaded animation index: %d (%s), brightness: %d\n", currentIndex, animations[currentIndex]->getName(), brightStep);
	} else {
		DBG_PRINTF("[AppManager] Loaded animation index: %d, brightness: %d\n", currentIndex, brightStep);
	}

    
	if (!animations.empty()) {
		if (currentIndex < 0 || currentIndex >= (int)animations.size()) currentIndex = 0;
		// Load animation-specific settings via StorageManager if supported
		storage.loadAnimation(*animations[currentIndex]);
		// notify animation that it's now active (for warmups, etc.)
		animations[currentIndex]->onActivate();
	}
	DBG_PRINTLN("[AppManager] Initialization complete");
	lastActivityMillis = millis();
	appState = powered ? STATE_RUNNING : STATE_POWER_OFF;
}

void AppManager::update() {
	unsigned long now = millis();

	// 1) Input
	updateInput(now);

	// 2) State
	updateState(now);

	// 3) Frame scheduling (non-blocking)
	unsigned long elapsed = (unsigned long)(now - lastFrameMillis);
	bool due = (elapsed >= frameIntervalMs);
	// Рендерим кадр либо по расписанию FPS, либо при активном оверлее/очистке
	shouldRender = due || needClearOnce || overlayPowerOffActive || overlayPowerOnActive || (!powered && !powered_off_shown);
	if (shouldRender) {
		if (due) {
			lastFrameMillis = now;
		}
		// 4) Base render (if applicable)
		renderBase();
		// 5) Overlay render (power-on/off etc.)
		renderOverlay();
		// 6) Show
		showFrame();
	}
}

void AppManager::updateInput(unsigned long now) {
	// В текущей архитектуре ввод приходит через onEvent().
	// Здесь можно добавить polling других входов (например, кнопки/датчики), если появятся.
	(void)now;
}

void AppManager::updateState(unsigned long now) {
	// Обработка удержания кнопки для выключения — без блокирующих return
	overlayPowerOffActive = false;
	overlayPowerOnActive = false;

	if (btnDown && powered) {
		unsigned long held = now - btnPressedMillis;
		if (held >= APP_POWEROFF_HOLD_MS) {
			// Завершение выключения: форсированный коммит всех отложенных изменений
			flushDirty(true);
			powered = false;
			mode = MODE_POWEROFF;
			saveState();
			DBG_PRINTLN("[AppManager] POWERED OFF - state saved to NVS");
			powered_off_shown = false;
			needClearOnce = true; // очистить при первом рендере после выключения
			overlayPowerOffActive = false; // оверлей выключения более не нужен
			appState = STATE_POWER_OFF;
		} else {
			// Активный оверлей выключения во время удержания
			int prog;
			if (held >= APP_POWEROFF_MIN_ANIM_MS) {
				prog = 255 - (int)((uint32_t)held * 255 / APP_POWEROFF_HOLD_MS);
				if (prog < 0) prog = 0;
			} else {
				prog = 255; // в начале удержания — все сегменты включены
			}
			powerOffAnim.setProgress((uint8_t)prog);
			overlayPowerOffActive = true;
		}
	}

	// Состояние после выключения питания — держим матрицу очищенной, но не блокируем цикл
	if (!powered) {
		overlayPowerOnActive = false;
		overlayPowerOffActive = false;
		// Очистим матрицу один раз, затем ничего не рисуем
		if (!powered_off_shown) {
			needClearOnce = true;
		}
		return; // логика ниже актуальна только при включенном питании
	}

	// Оверлей включения питания (power-on) — непродолжительная анимация
	if ((long)(now - powerOnUntilMs) < 0) {
		unsigned long elapsedOn = now - powerOnStartMillis;
		if (elapsedOn > (unsigned long)APP_POWERON_ANIM_MS) elapsedOn = (unsigned long)APP_POWERON_ANIM_MS;
		uint8_t progOn = (uint8_t)((elapsedOn * 255UL) / (unsigned long)APP_POWERON_ANIM_MS);
		powerOnAnim.setProgress(progOn);
		overlayPowerOnActive = true;
		appState = STATE_POWER_ON;
	}

	// Автопереход в режим яркости при простое
	if ((mode == MODE_SELECT_ANIM || mode == MODE_COLOR) && (now - lastActivityMillis >= APP_IDLE_TIMEOUT_MS)) {
		mode = MODE_BRIGHTNESS;
		DBG_PRINTLN("[AppManager] Auto-switch to BRIGHTNESS due to inactivity");
		lastActivityMillis = now;

		// Если питание включено и нет оверлеев — обычная работа
		if (powered && !overlayPowerOnActive && !overlayPowerOffActive) {
			appState = STATE_RUNNING;
		}
	}

	// Deferred save: coalesce app and animation changes
	unsigned long firstDirtyMs = 0;
	if (stateDirty) firstDirtyMs = stateDirtySinceMs;
	for (size_t i = 0; i < animDirty.size(); ++i) {
		if (animDirty[i]) { if (firstDirtyMs == 0 || animDirtySinceMs < firstDirtyMs) firstDirtyMs = animDirtySinceMs; break; }
	}
	if (firstDirtyMs != 0 && (unsigned long)(now - firstDirtyMs) >= (unsigned long)APP_SAVE_DEFER_MS) {
		flushDirty(false);
	}
}

void AppManager::renderBase() {
	if (!matrix) return;

	if (!powered) {
		// При выключенном питании базовый рендер не выполняется
		return;
	}

	if (animations.empty()) return;
	if (currentIndex < 0 || currentIndex >= (int)animations.size()) return;
	// Если активен любой оверлей — базовую анимацию не рисуем
	if (overlayPowerOnActive || overlayPowerOffActive) return;
	switch (appState) {
		case STATE_POWER_OFF:
			return;
		case STATE_POWER_ON:
			return;
		case STATE_RUNNING:
			if (!powered) return;
			if (mode == MODE_SELECT_ANIM || mode == MODE_BRIGHTNESS || mode == MODE_COLOR) {
				animations[currentIndex]->render(*matrix);
			}
			return;
	}
}

void AppManager::renderOverlay() {
	if (!matrix) return;

	// Оверлей выключения имеет приоритет и выводится поверх (с очисткой)
	// Оверлей включения — поверх базовой
	switch (appState) {
		case STATE_POWER_OFF:
			// При полностью выключенном состоянии ничего не рисуем поверх.
			// Очистка кадра выполняется в showFrame() через needClearOnce.
			return;
		case STATE_POWER_ON:
			// Оверлей включения — поверх кадра, обычно с очисткой
			matrix->clear();
			powerOnAnim.render();
			return;
		case STATE_RUNNING:
			// Оверлей выключения при удержании — поверх базы
			if (overlayPowerOffActive) {
				matrix->clear();
				powerOffAnim.render();
			}
			return;
	}
}

void AppManager::showFrame() {
	if (!matrix) return;

	if (needClearOnce) {
		matrix->clear();
		needClearOnce = false;
		powered_off_shown = true;
	}

	matrix->show();
}

void AppManager::onEvent(RotaryEncoder::Event ev, int value) {
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
				DBG_PRINTLN("[AppManager] POWERED ON - state restored");

				// restore current animation settings (if any)
				if (!animations.empty() && currentIndex >= 0 && currentIndex < (int)animations.size()) {
					storage.loadAnimation(*animations[currentIndex]);
					// notify animation that it's now active (for warmups, etc.)
					animations[currentIndex]->onActivate();
				}

				// Синхронизацию границ/значения энкодера больше не выполняем (фиксация на старте)

				// trigger power-on animation
				powerOnStartMillis = millis();
				powerOnUntilMs = powerOnStartMillis + (unsigned long)APP_POWERON_ANIM_MS;
				appState = STATE_POWER_ON;
			}
			return;
		}

		if (held >= APP_POWEROFF_HOLD_MS) return;

		// short press: cycle modes (Brightness -> Select Animation -> Color)
		if (mode == MODE_COLOR) {
			mode = MODE_BRIGHTNESS;
			DBG_PRINTLN("[AppManager] Mode: BRIGHTNESS");
		} else if (mode == MODE_BRIGHTNESS) {
			mode = MODE_SELECT_ANIM;
			DBG_PRINTLN("[AppManager] Mode: SELECT_ANIM");
		} else if (mode == MODE_SELECT_ANIM) {
			mode = MODE_COLOR;
			DBG_PRINTLN("[AppManager] Mode: COLOR");
		} else {
			mode = MODE_BRIGHTNESS;
			DBG_PRINTLN("[AppManager] Mode: BRIGHTNESS");
		}

		return;
	}

	// rotation: используем дельту энкодера относительно базового значения
	if (ev == RotaryEncoder::INCREMENT || ev == RotaryEncoder::DECREMENT) {
		if (!powered || btnDown) return;

		// корректируем дельту с учётом wrap
		int delta = value - encBaseValue;
		if (delta > ENC_HALF) delta -= ENC_RANGE;
		else if (delta < -ENC_HALF) delta += ENC_RANGE;
		if (delta == 0) return;
		encBaseValue = value;

		switch (mode) {
			case MODE_SELECT_ANIM:
				if (animations.empty() || !matrix) return;
				{
					int n = (int)animations.size();
					int newIndex = currentIndex + delta;
					// wrap в пределах списка анимаций
					while (newIndex < 0) newIndex += n;
					while (newIndex >= n) newIndex -= n;
					if (newIndex == currentIndex) return;

					matrix->clear();
					matrix->show();

					currentIndex = newIndex;
					DBG_PRINTF("[AppManager] Animation changed to: %d (%s)\n", currentIndex, animations[currentIndex]->getName());
						storage.loadAnimation(*animations[currentIndex]);
					animations[currentIndex]->onActivate();
				}
				break;

			case MODE_BRIGHTNESS:
				brightStep = constrain(brightStep + delta, 0, APP_STEPS - 1);
				applyMasterBrightness();
				scheduleStateSave();
				DBG_PRINTF("[AppManager] Brightness: %d/%d\n", brightStep, APP_STEPS);
				break;

			case MODE_COLOR:
				colorStep = constrain(colorStep + delta, 0, APP_COLOR_STEPS - 1);
				if (!matrix || animations.empty()) break;
				if (currentIndex < 0 || currentIndex >= (int)animations.size()) break;
				{
					int hue = (colorStep * 256) / APP_COLOR_STEPS;
					if (hue > 255) hue = 255;
						animations[currentIndex]->setColorHSV((uint8_t)hue, 255);
					// mark animation config dirty for deferred save
					if (currentIndex >= 0 && currentIndex < (int)animDirty.size()) {
						animDirty[(size_t)currentIndex] = 1;
						animDirtySinceMs = millis();
					}
					DBG_PRINTF("[AppManager] Color (Hue): %d (%d/%d)\n", hue, colorStep, APP_COLOR_STEPS);
				}
				break;

			case MODE_POWEROFF:
				break;
		}
	}
}

bool AppManager::saveState() {
	// Prepare AppCfg from current runtime state
	appCfg.masterBrightness = (uint16_t)constrain(brightStep, 0, APP_STEPS - 1);
	if (!animations.empty() && currentIndex >= 0 && currentIndex < (int)animations.size()) {
		appCfg.lastAnimId = animations[currentIndex]->getId();
	} else {
		appCfg.lastAnimId = 0;
	}
	bool ok = storage.saveApp(appCfg);
	if (ok) {
		DBG_PRINTF("[NVS] State saved: animId=%u, brightnessStep=%d\n", (unsigned)appCfg.lastAnimId, brightStep);
	} else {
		DBG_PRINTLN("[NVS] Error: Failed to save app state");
	}
	return ok;
}

bool AppManager::loadState() {
	// Load AppCfg via StorageManager
	bool ok = storage.loadApp(appCfg);
	// Apply brightness (clamp)
	brightStep = (int)appCfg.masterBrightness;
	if (brightStep < 0) brightStep = 0;
	if (brightStep >= APP_STEPS) brightStep = APP_STEPS - 1;
	// Resolve animation index by stored ID
	int resolved = 0;
	if (!animations.empty() && appCfg.lastAnimId != 0) {
		for (size_t i = 0; i < animations.size(); ++i) {
			if (animations[i]->getId() == appCfg.lastAnimId) {
				resolved = (int)i;
				break;
			}
		}
	}
	currentIndex = resolved;
	DBG_PRINTF("[NVS] State loaded: animId=%u -> index=%d, brightnessStep=%d\n", (unsigned)appCfg.lastAnimId, currentIndex, brightStep);
	return ok;
}

void AppManager::applyMasterBrightness() {
	int bs = constrain(brightStep, 0, APP_STEPS - 1);
	if (!matrix) return;
	// Gamma LUT mapping (fallback to linear if LUT not ready)
	uint8_t v8 = 0;
	if (!gammaLUT.empty() && bs >= 0 && bs < (int)gammaLUT.size()) {
		v8 = gammaLUT[bs];
	} else {
		// Fallback linear (should not normally happen)
		unsigned long v = (APP_STEPS > 1) ? ((unsigned long)bs * 255UL / (unsigned long)(APP_STEPS - 1)) : 255UL;
		if (v > 255UL) v = 255UL;
		v8 = (uint8_t)v;
	}
	matrix->setMasterBrightness(v8);
}

void AppManager::flushDirty(bool force) {
	bool did = false;
	// save app state
	if (stateDirty || force) {
		if (saveState()) {
			DBG_PRINTLN("[AppManager] State saved (flush)");
		} else {
			DBG_PRINTLN("[AppManager] State save failed (flush)");
		}
		stateDirty = false;
		did = true;
	}
	// save animations configs
	for (size_t i = 0; i < animDirty.size(); ++i) {
		if ((i < animations.size()) && (animDirty[i] || force)) {
			storage.saveAnimation(*animations[i]);
			animDirty[i] = 0;
			did = true;
		}
	}
	if (did) {
		animDirtySinceMs = 0;
	}
}

// Removed ISerializable implementation from AppManager; AppCfg handles serialization.