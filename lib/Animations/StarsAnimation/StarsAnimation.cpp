#include "StarsAnimation.hpp"
#include <FastLED.h>

StarsAnimation::StarsAnimation(LedMatrix& m)
	: AnimationBase(m, STARS_DEFAULT_HUE, STARS_DEFAULT_SAT) {
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
		// make movement slower (smaller velocities) for smoother motion
		int8_t baseV = (s.depth == 0) ? -8 : (s.depth == 1) ? -14 : -24;
		// случайно меняем направление для разнообразия
		if ((uint8_t)random(0, 2)) baseV = -baseV;
		s.vfp = baseV;
		// Плавное мерцание: индивидуальная скорость (медленная) и случайная фаза
		s.twPhase = (uint8_t)random(0, 256);
		// slower twinkle speeds for smoother effect
		s.twSpeed = (uint8_t)((s.depth == 0) ? 1 : (s.depth == 1) ? 1 : 2);
		stars.push_back(s);
	}
	// Остальные — случайно по всей матрице
	for (int i = base; i < starCount; ++i) {
		Star s;
		randomizeStar(s);
		stars.push_back(s);
	}
}

// Base class provides ISerializable

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
	// slower lateral movement
	int8_t baseV = (s.depth == 0) ? -8 : (s.depth == 1) ? -14 : -24;
	if ((uint8_t)random(0, 2)) baseV = -baseV;
	s.vfp = baseV;
	// Плавное мерцание: фаза и скорость
	s.twPhase = (uint8_t)random(0, 256);
	s.twSpeed = (uint8_t)((s.depth == 0) ? 1 : (s.depth == 1) ? 1 : 2);
}

void StarsAnimation::render() {
	uint32_t now = millis();
	// очистка матрицы перед рисованием (контроллер задаёт частоту вызова render)
	if (!matrix) return;
	matrix->clear();

	// размеры для обёртки и фикс-точки
	int w = matrix->width();
	int h = matrix->height();
	if (w <= 0) w = 8;
	if (h <= 0) h = 8;
	int w8 = w << 8;

	// dt в миллисекундах с предохранением на первый кадр
	uint32_t dt = 0;
	if (lastMillis == 0) dt = 16;
	else dt = now - lastMillis;

	for (auto &s : stars) {
		// Обновляем целевую яркость с помощью sin8 + noise8 для естественности
		// noise per-star gives spatial variance
		uint8_t noise = inoise8((uint16_t)s.x * 17u + (uint16_t)s.y * 29u);
		// tw is 0..255
		uint8_t tw = sin8((uint8_t)(s.twPhase) + noise);
		// scale tw by master val to get a target in 0..val
		uint8_t computedTarget = scale8(tw, (uint8_t)ANIMATION_DEFAULT_VAL);
		// occasionally refresh target timing
		if (now >= s.nextChangeMillis) {
			s.target = computedTarget;
			s.nextChangeMillis = now + (unsigned long)random(100, 1500);
		}

		// Обновляем позицию по X с учётом параллакса и dt (fixed-point)
		// slower movement: increase divisor so per-ms movement is smaller
		int32_t delta = ((int32_t)s.vfp * (int32_t)dt) / 64; // smoother, slower motion
		s.xfp = (int16_t)(s.xfp + delta);
		// корректный wrap в пределах 0..w8-1
		while (s.xfp >= w8) s.xfp -= w8;
		while (s.xfp < 0) s.xfp += w8;
		s.x = (uint8_t)(s.xfp >> 8);

		// Обновляем мерцание (brightness -> target) малыми шагами, зависящими от dt
		if (s.brightness < s.target) {
			uint16_t diff = (uint16_t)(s.target - s.brightness);
			// slower smoothing: larger divisor for dt
			uint16_t step = (dt / 200u) + 1u; // much slower approach
			if (step > diff) step = diff;
			s.brightness = (uint8_t)(s.brightness + (uint8_t)step);
		} else if (s.brightness > s.target) {
			uint16_t diff = (uint16_t)(s.brightness - s.target);
			uint16_t step = (dt / 200u) + 1u;
			if (step > diff) step = diff;
			s.brightness = (uint8_t)(s.brightness - (uint8_t)step);
		}

		// advance twinkle phase
		s.twPhase = (uint8_t)(s.twPhase + s.twSpeed);

		// Сделаем звезды заметнее: используем линейную яркость и ниже порог
		uint8_t baseV = s.brightness;
		uint8_t depthGain = (s.depth == 0) ? 180 : (s.depth == 1) ? 220 : 255;
		baseV = scale8(baseV, depthGain);
		// soften with tw (already used for target) — apply small modulation
		// milder twinkle modulation (slower/smoother)
		uint8_t twMod = (uint8_t)(160 + (tw >> 3));
		baseV = scale8(baseV, twMod);
		uint8_t drawV = scale8((uint8_t)ANIMATION_DEFAULT_VAL, baseV);
		if (drawV < 12) continue; // отсечём совсем слабые
		// slight hue shift by depth for parallax color
		uint8_t starHue = (uint8_t)(animCfg.hue + (s.depth == 2 ? 0 : (s.depth == 1 ? 4 : 8)));
	matrix->setPixelHSV(s.x, s.y, starHue, animCfg.sat, drawV);
	}

	lastMillis = now;

	// show() is managed by AppController
}
