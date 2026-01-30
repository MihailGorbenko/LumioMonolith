#ifndef POWEROFF_ANIMATION_HPP
#define POWEROFF_ANIMATION_HPP

#include <Arduino.h>
#include "../LedMatrix/LedMatrix.hpp"
#include "../Animation/OverlayAnimation.hpp"

// default color (можно переопределить в проекте)
#ifndef POWEROFF_DEFAULT_HUE
#define POWEROFF_DEFAULT_HUE 170   // синий
#endif
#ifndef POWEROFF_DEFAULT_SAT
#define POWEROFF_DEFAULT_SAT 255
#endif
#ifndef POWEROFF_DEFAULT_VAL
#define POWEROFF_DEFAULT_VAL 255
#endif

class PowerOffAnimation : public OverlayAnimation {
public:
	explicit PowerOffAnimation(LedMatrix& m);

	// контроллер устанавливает прогресс 0..255
	void setProgress(uint8_t p);

	// отрисовать кадр (вызывать контроллером с нужной частотой)
	void render() override;

private:
	LedMatrix* matrix;
	uint8_t hue;
	uint8_t sat;
	uint8_t val;
	uint8_t progress; // 0..255
};

#endif // POWEROFF_ANIMATION_HPP

