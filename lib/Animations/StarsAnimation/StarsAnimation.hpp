#ifndef STARS_ANIMATION_HPP
#define STARS_ANIMATION_HPP

#include <Arduino.h>
#include <vector>
#include "../Animation/Animation.hpp"

// default configuration (можно переопределить в проекте перед инклюдом)
#ifndef STARS_DEFAULT_HUE
#define STARS_DEFAULT_HUE 0
#endif
#ifndef STARS_DEFAULT_SAT
#define STARS_DEFAULT_SAT 255
#endif
#ifndef STARS_DEFAULT_VAL
#define STARS_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif
#ifndef STARS_STAR_COUNT
#define STARS_STAR_COUNT 20
#endif


class StarsAnimation : public AnimationBase {
public:
	// принимает только матрицу; прочие параметры — дефайнами
	explicit StarsAnimation(LedMatrix& m);

	// унаследованный метод: установить цвет (base хранит hue/sat/val)
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;

	// отрисовка кадра — вызывать часто из loop()
	void render() override;

	// сохраняет/загружает цвет анимации в NVS под заданным ключом
	bool saveColor(const char* key);
	bool loadColor(const char* key);

private:
	struct Star {
		uint8_t x;
		uint8_t y;
		uint8_t brightness;    // текущее значение яркости (0..255)
		uint8_t target;        // целевая яркость
		unsigned long nextChangeMillis;
	};

	std::vector<Star> stars;
	int starCount;
	

	// вспомогательные
	void randomizeStar(Star& s);
};

#endif // STARS_ANIMATION_HPP
