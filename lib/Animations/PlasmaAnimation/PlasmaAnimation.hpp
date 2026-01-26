#ifndef PLASMA_ANIMATION_HPP
#define PLASMA_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

#ifndef PLASMA_DEFAULT_HUE
#define PLASMA_DEFAULT_HUE 160
#endif
#ifndef PLASMA_DEFAULT_SAT
#define PLASMA_DEFAULT_SAT 255
#endif
#ifndef PLASMA_DEFAULT_VAL
#define PLASMA_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

// Readable name
#ifndef PLASMA_ANIMATION_NAME
#define PLASMA_ANIMATION_NAME "Plasma"
#endif

class PlasmaAnimation : public AnimationBase, public ISerializable {
public:
	explicit PlasmaAnimation(LedMatrix& m);
	void render() override;

	// ISerializable
	size_t serializedSize() const override { return 2; }
	bool serialize(uint8_t* out, size_t maxLen) const override;
	bool deserialize(const uint8_t* data, size_t len) override;
	ISerializable* serializable() override { return this; }
};

#endif // PLASMA_ANIMATION_HPP
