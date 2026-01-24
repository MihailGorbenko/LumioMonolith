#include "Animation.hpp"
#include <Preferences.h>

// Debug mode for NVS operations
#ifndef ANIM_NVS_DEBUG
#define ANIM_NVS_DEBUG 1
#endif

#if ANIM_NVS_DEBUG
#define ANIM_DBG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define ANIM_DBG_PRINTF(...)
#endif

static void makeKey(char* out, size_t outSize, const char* key, const char* suffix) {
	if (!out || outSize == 0) return;
	if (!key) {
		out[0] = '\0';
		return;
	}
	snprintf(out, outSize, "%s%s", key, suffix);
	out[outSize - 1] = '\0';
}

bool AnimationBase::saveToNVS(const char* key) {
	if (!key) {
		ANIM_DBG_PRINTF("[NVS-Anim] Error: Null key in saveToNVS\n");
		return false;
	}

	Preferences prefs;
	if (!prefs.begin("anim", false)) {
		ANIM_DBG_PRINTF("[NVS-Anim] Error: Cannot begin write (key=%s)\n", key);
		return false;
	}

	char kh[32], ks[32];
	makeKey(kh, sizeof(kh), key, "_h");
	makeKey(ks, sizeof(ks), key, "_s");

	bool ok = true;
	ok &= (prefs.putUChar(kh, hue) != 0);
	ok &= (prefs.putUChar(ks, sat) != 0);
	prefs.end();

	ANIM_DBG_PRINTF("[NVS-Anim] Saved (key=%s): H=%d S=%d (V ignored; global via masterBrightness)\n", key, hue, sat);
	return ok;
}

bool AnimationBase::loadFromNVS(const char* key) {
	if (!key) {
		ANIM_DBG_PRINTF("[NVS-Anim] Error: Null key in loadFromNVS\n");
		return false;
	}

	Preferences prefs;
	if (!prefs.begin("anim", true)) {
		// apply defaults through setter so subclasses see them
		setColorHSV(defaultHue, defaultSat, ANIMATION_DEFAULT_VAL);
		ANIM_DBG_PRINTF("[NVS-Anim] Warning: Cannot read (key=%s), using defaults\n", key);
		return false;
	}

	char kh[32], ks[32];
	makeKey(kh, sizeof(kh), key, "_h");
	makeKey(ks, sizeof(ks), key, "_s");

	uint8_t h = prefs.getUChar(kh, defaultHue);
	uint8_t s = prefs.getUChar(ks, defaultSat);
	prefs.end();

	// Force V to global default (255) so brightness is controlled only by masterBrightness
	setColorHSV(h, s, ANIMATION_DEFAULT_VAL);

	ANIM_DBG_PRINTF("[NVS-Anim] Loaded (key=%s): H=%d S=%d (V forced to 255)\n", key, h, s);
	return true;
}
