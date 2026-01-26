#ifndef PULSE_WAVE_ANIMATION_HPP
#define PULSE_WAVE_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef PULSEWAVE_DEFAULT_HUE
#define PULSEWAVE_DEFAULT_HUE 200  // cyan
#endif
#ifndef PULSEWAVE_DEFAULT_SAT
#define PULSEWAVE_DEFAULT_SAT 255
#endif
#ifndef PULSEWAVE_DEFAULT_VAL
#define PULSEWAVE_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Readable name
#ifndef PULSEWAVE_ANIMATION_NAME
#define PULSEWAVE_ANIMATION_NAME "Pulse Wave"
#endif

class PulseWaveAnimation : public AnimationBase, public ISerializable {
public:
	explicit PulseWaveAnimation(LedMatrix& m);
	void render() override;

private:
	uint8_t pulseRadius;  // 0..255

	// ISerializable
public:
	size_t serializedSize() const override { return 2; }
	bool serialize(uint8_t* out, size_t maxLen) const override;
	bool deserialize(const uint8_t* data, size_t len) override;
	ISerializable* serializable() override { return this; }
};

#endif // PULSE_WAVE_ANIMATION_HPP
