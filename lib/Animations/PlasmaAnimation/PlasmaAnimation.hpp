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

class PlasmaAnimation : public AnimationBase {
public:
	explicit PlasmaAnimation(uint16_t id, LedMatrix& m);
	void render() override;
	const char* getName() const override { return PLASMA_ANIMATION_NAME; }
};

#endif // PLASMA_ANIMATION_HPP
