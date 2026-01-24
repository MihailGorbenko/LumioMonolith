#include "StarsAnimation.hpp"
#include <FastLED.h>

StarsAnimation::StarsAnimation(LedMatrix& m)
	: AnimationBase(m, STARS_DEFAULT_HUE, STARS_DEFAULT_SAT, STARS_DEFAULT_VAL) {
	// вычисляем количество звёзд автоматически по размеру матрицы (~40% пикселей)
	int w = 0, h = 0;
	if (matrix) {
		w = matrix->width();
		h = matrix->height();
	}
	if (w <= 0) w = 8;
	if (h <= 0) h = 8;
	int total_pixels = w * h;
	int autoCount = max(1, (total_pixels * 3) / 5); // ~60% пикселей
	starCount = autoCount;

	stars.reserve(starCount);
	// Обеспечим покрытие ширины равномерно распределёнными X для части звёзд
	int base = min(w, starCount);
	for (int i = 0; i < base; ++i) {
		Star s;
		s.x = (uint8_t)((long)i * w / base);
		s.y = (uint8_t)random(0, h);
		s.brightness = (uint8_t)random(0, 256);
		s.target = (uint8_t)random(0, 256);
		s.nextChangeMillis = millis() + (unsigned long)random(400, 2200); // медленнее смена цели
		// Параллакс: назначаем глубину и скорость
		s.depth = (uint8_t)random(0, 3); // 0..2
		s.xfp = ((int16_t)s.x) << 8;
		// движение влево/вправо, ближние быстрее, но в целом медленнее
		int8_t baseV = (s.depth == 0) ? -16 : (s.depth == 1) ? -28 : -40;
		// случайно меняем направление для разнообразия
		if ((uint8_t)random(0, 2)) baseV = -baseV;
		s.vfp = baseV;
		// Плавное мерцание: индивидуальная скорость (медленная) и случайная фаза
		s.twPhase = (uint8_t)random(0, 256);
		s.twSpeed = (uint8_t)((s.depth == 0) ? 1 : (s.depth == 1) ? 2 : 3); // ближние чуть быстрее
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
	s.nextChangeMillis = millis() + random(400, 2200); // медленнее смена цели
	// Параллакс: глубина, начальная фикспозиция, скорость
	s.depth = (uint8_t)random(0, 3);
	s.xfp = ((int16_t)s.x) << 8;
	int8_t baseV = (s.depth == 0) ? -16 : (s.depth == 1) ? -28 : -40;
	if ((uint8_t)random(0, 2)) baseV = -baseV;
	s.vfp = baseV;
	// Плавное мерцание: фаза и скорость
	s.twPhase = (uint8_t)random(0, 256);
	s.twSpeed = (uint8_t)((s.depth == 0) ? 1 : (s.depth == 1) ? 2 : 3);
}

void StarsAnimation::render() {
	unsigned long now = millis();
	// очистка матрицы перед рисованием (контроллер задаёт частоту вызова render)
	matrix->clear();

	// размеры для обёртки и фикс-точки
	int w = matrix->width();
	int h = matrix->height();
	if (w <= 0) w = 8;
	if (h <= 0) h = 8;
	int w8 = w << 8;

	for (auto &s : stars) {
		if (now >= s.nextChangeMillis) {
			s.target = random(0, (int)val + 1);
			s.nextChangeMillis = now + random(100, 1500);
		}

		// Обновляем позицию по X с учётом параллакса
		s.xfp = (int16_t)(s.xfp + s.vfp);
		if (s.xfp >= w8) s.xfp -= w8;
		else if (s.xfp < 0) s.xfp += w8;
		s.x = (uint8_t)(s.xfp >> 8);

		// Обновляем мерцание (brightness -> target) очень мелкими шагами
		if (s.brightness < s.target) {
			uint8_t diff = (uint8_t)(s.target - s.brightness);
			uint8_t delta = (diff > 1) ? 1 : diff; // ещё медленнее
			s.brightness = s.brightness + delta;
		} else if (s.brightness > s.target) {
			uint8_t diff = (uint8_t)(s.brightness - s.target);
			uint8_t delta = (diff > 1) ? 1 : diff;
			s.brightness = s.brightness - delta;
		}

		// Синусоидальная модуляция: медленная фаза для мягкого мерцания
		s.twPhase = (uint8_t)(s.twPhase + s.twSpeed);
		uint8_t tw = sin8(s.twPhase); // 0..255

		// Сделаем звезды заметнее: используем линейную яркость и ниже порог
		// Базовая яркость
		uint8_t baseV = s.brightness;
		// Параллакс-яркость: дальние слои тусклее, ближние ярче
		uint8_t depthGain = (s.depth == 0) ? 180 : (s.depth == 1) ? 220 : 255;
		baseV = scale8(baseV, depthGain);
		// Плавная синус-модуляция (ослабляющая): 160..224 коэффициент
		uint8_t twMod = (uint8_t)(160 + (tw >> 2));
		baseV = scale8(baseV, twMod);
		uint8_t drawV = scale8(val, baseV);
		if (drawV < 12) continue; // отсечём совсем слабые
		matrix->setPixelHSV(s.x, s.y, hue, sat, drawV);
	}

	matrix->show();
}
