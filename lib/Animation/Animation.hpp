#ifndef ANIMATION_HPP
#define ANIMATION_HPP
#include <Arduino.h>
#include <cstring>
#include "AnimConfig.hpp"
class LedMatrix; // forward declaration to avoid heavy include dependency

// Default master brightness for all animations (final multiplier)
#ifndef ANIMATION_DEFAULT_VAL
#define ANIMATION_DEFAULT_VAL 255
#endif

// Default global saturation (not stored in animation config)
#ifndef ANIMATION_DEFAULT_SAT
#define ANIMATION_DEFAULT_SAT 255
#endif

// Base animation class: stores color config (hue only); matrix passed to render
class AnimationBase {
protected:
	AnimConfig animCfg;
	uint16_t animId;
	bool configDirty;
	bool initialized;
	LedMatrix& matrix;

public:
	// Constructor: default hue and animation ID (matrix passed to render)
	explicit AnimationBase(uint8_t defH, uint16_t id, LedMatrix& m);

	// Set hue (0..255). Non-virtual for frequent calls.
	// Marks config dirty when changed.
	inline void setHue(uint8_t h) {
		if (h != animCfg.hue) {
			animCfg.setHue(h);
			configDirty = true;
		}
	}
	// Return true if the animation config was modified since last clear
	inline bool isConfigDirty() const { return configDirty; }

	// Clear the config-dirty flag
	inline void clearConfigDirty() { configDirty = false; }

	// Mark the config as dirty (external callers)
	inline void setConfigDirty() { configDirty = true; }

	// Access to animation config (hue only)
	inline const AnimConfig& getConfig() const { return animCfg; }
	inline AnimConfig& getConfig() { return animCfg; }



	// Get stable animation ID
	inline uint16_t getId() const { return animId; }

	// (Matrix is provided via constructor; no setter provided)

	// Return whether the animation has been initialized
	inline bool isInitialized() const { return initialized; }

	// Animation name defined by subclasses (abstract)
	virtual const char* getName() const = 0;

	// Form short NVS key by ID (format: "a<id>")
	// Fits NVS limit (<=15 chars). Writes NUL-terminated string to out.
	void makeNvsKeyById(char* out, size_t outSize) const;

	// Subclasses implement animation logic; use internal matrix pointer
	virtual void render() = 0;

	// Hook called on activation; default marks the animation initialized
	virtual void onActivate() { initialized = true; }

};
#endif // ANIMATION_HPP
