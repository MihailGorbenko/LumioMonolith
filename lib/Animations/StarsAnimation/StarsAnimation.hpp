#ifndef STARS_ANIMATION_HPP
#define STARS_ANIMATION_HPP

#include <Arduino.h>
#include <vector>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

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

// Readable name
#ifndef STARS_ANIMATION_NAME
#define STARS_ANIMATION_NAME "Stars"
#endif


class StarsAnimation : public AnimationBase {
public:
	// default hue comes from defines; id and matrix are provided by main
	explicit StarsAnimation(uint16_t id, LedMatrix& m);

	// render one frame
	void render() override;

	const char* getName() const override { return STARS_ANIMATION_NAME; }

private:
	struct Star {
		uint8_t x;
		uint8_t y;
		uint8_t brightness;    // текущее значение яркости (0..255)
		uint8_t target;        // целевая яркость
		unsigned long nextChangeMillis;
		// Параллакс: слой глубины и субпиксельная позиция/скорость по X
		uint8_t depth;          // 0=далёкие (медленно), 1=средние, 2=ближние (быстро)
		int16_t xfp;            // фикс-точка X (8.8), x = xfp>>8
		int8_t vfp;             // скорость по X в фикс-точке (px*256/кадр)
		// Плавное мерцание: фаза и скорость синусоидального мода
		uint8_t twPhase;        // 0..255
		uint8_t twSpeed;        // маленькие значения для медленного мерцания
	};

	std::vector<Star> stars;
	int starCount;
	// время последнего кадра для dt-зависимого движения
	uint32_t lastMillis = 0;


	// вспомогательные
	void randomizeStar(Star& s, int w, int h);
};

#endif // STARS_ANIMATION_HPP
