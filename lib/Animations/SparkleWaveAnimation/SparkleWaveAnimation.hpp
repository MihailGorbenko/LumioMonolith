#ifndef SPARKLE_WAVE_ANIMATION_HPP
#define SPARKLE_WAVE_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef SPARKLEWAVE_DEFAULT_HUE
#define SPARKLEWAVE_DEFAULT_HUE 96
#endif
#ifndef SPARKLEWAVE_DEFAULT_SAT
#define SPARKLEWAVE_DEFAULT_SAT 255
#endif
#ifndef SPARKLEWAVE_DEFAULT_VAL
#define SPARKLEWAVE_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Readable name
#ifndef SPARKLEWAVE_ANIMATION_NAME
#define SPARKLEWAVE_ANIMATION_NAME "Sparkle Wave"
#endif

class SparkleWaveAnimation : public AnimationBase, public ISerializable {
public:
	explicit SparkleWaveAnimation(LedMatrix& m);
	void render() override;

private:
	uint8_t sparkleChance; // 0..255

	// ISerializable
public:
	size_t serializedSize() const override { return 2; }
	bool serialize(uint8_t* out, size_t maxLen) const override;
	bool deserialize(const uint8_t* data, size_t len) override;
	ISerializable* serializable() override { return this; }
};

#endif // SPARKLE_WAVE_ANIMATION_HPP
