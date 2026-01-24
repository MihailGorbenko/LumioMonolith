#include "StarsAnimation.hpp"

StarsAnimation::StarsAnimation(LedMatrix& m)
	: AnimationBase(m, STARS_DEFAULT_HUE, STARS_DEFAULT_SAT, STARS_DEFAULT_VAL) {
	// вычисляем количество звёзд автоматически по размеру матрицы (~20% пикселей),
	// но не больше STARS_STAR_COUNT
	int w = 0, h = 0;
	if (matrix) {
		w = matrix->width();
		h = matrix->height();
	}
	if (w <= 0) w = 8;
	if (h <= 0) h = 8;
	int total_pixels = w * h;
	int autoCount = max(1, total_pixels / 5); // ~20% пикселей
	starCount = autoCount; // делаем реже, чтобы повысить контраст

	stars.reserve(starCount);
	// Обеспечим покрытие ширины равномерно распределёнными X для части звёзд
	int base = min(w, starCount);
	for (int i = 0; i < base; ++i) {
		Star s;
		s.x = (uint8_t)((long)i * w / base);
		s.y = (uint8_t)random(0, h);
		s.brightness = (uint8_t)random(0, 256);
		s.target = (uint8_t)random(0, 256);
		s.nextChangeMillis = millis() + (unsigned long)random(100, 1500);
		stars.push_back(s);
	}
	// Остальные — случайно по всей матрице
	for (int i = base; i < starCount; ++i) {
		Star s;
		randomizeStar(s);
		stars.push_back(s);
	}
}

void StarsAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
	AnimationBase::setColorHSV(h, s, v);
}

bool StarsAnimation::saveColor(const char* key) {
	return saveToNVS(key);
}

bool StarsAnimation::loadColor(const char* key) {
	return loadFromNVS(key);
}

void StarsAnimation::randomizeStar(Star& s) {
	uint8_t w = 8;
	uint8_t h = 8;
	// получить реальные размеры из матрицы, если доступны
	if (matrix) {
		// предполагаем методы width() и height() в LedMatrix
		w = (uint8_t)matrix->width();
		h = (uint8_t)matrix->height();
		// защита от нуля
		if (w == 0) w = 8;
		if (h == 0) h = 8;
	}
	s.x = random(0, w);
	s.y = random(0, h);
	s.brightness = random(0, 256);
	s.target = random(0, 256);
	s.nextChangeMillis = millis() + random(100, 1500);
}

void StarsAnimation::render() {
	unsigned long now = millis();
	// очистка матрицы перед рисованием (контроллер задаёт частоту вызова render)
	matrix->clear();

	for (auto &s : stars) {
		if (now >= s.nextChangeMillis) {
			s.target = random(0, (int)val + 1);
			s.nextChangeMillis = now + random(100, 1500);
		}

		if (s.brightness < s.target) {
			uint8_t delta = min(8, s.target - s.brightness);
			s.brightness = s.brightness + delta;
		} else if (s.brightness > s.target) {
			uint8_t delta = min(8, s.brightness - s.target);
			s.brightness = s.brightness - delta;
		}

		// Повышаем контраст: нелинейно усиливаем яркие пики и гасим слабые
		uint8_t shaped = scale8(s.brightness, s.brightness); // ~ b^2 / 255
		uint8_t drawV = scale8(val, shaped);
		if (drawV < 24) continue; // отсечём слабое свечение
		matrix->setPixelHSV(s.x, s.y, hue, sat, drawV);
	}

	matrix->show();
}
