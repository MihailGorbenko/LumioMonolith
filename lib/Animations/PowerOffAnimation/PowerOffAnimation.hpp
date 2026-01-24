#ifndef POWEROFF_ANIMATION_HPP
#define POWEROFF_ANIMATION_HPP

#include <Arduino.h>
#include "../LedMatrix/LedMatrix.hpp"

// default color (можно переопределить в проекте)
#ifndef POWEROFF_DEFAULT_HUE
#define POWEROFF_DEFAULT_HUE 85   // зелёный
#endif
#ifndef POWEROFF_DEFAULT_SAT
#define POWEROFF_DEFAULT_SAT 255
#endif
#ifndef POWEROFF_DEFAULT_VAL
#define POWEROFF_DEFAULT_VAL 255
#endif

class PowerOffAnimation {
public:
	explicit PowerOffAnimation(LedMatrix& m);

	// контроллер устанавливает прогресс 0..255
	void setProgress(uint8_t p);

	// заглушки для установки/сохранения цвета (цвет динамический/фиксированный)
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v);
	bool saveColor(const char* key);
	bool loadColor(const char* key);

	// отрисовать кадр (вызывать контроллером с нужной частотой)
	void render();

private:
	LedMatrix* matrix;
	uint8_t hue;
	uint8_t sat;
	uint8_t val;
	uint8_t progress; // 0..255
};

#endif // POWEROFF_ANIMATION_HPP

