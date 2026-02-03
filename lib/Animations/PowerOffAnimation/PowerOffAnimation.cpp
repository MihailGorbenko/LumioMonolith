#include "PowerOffAnimation.hpp"

PowerOffAnimation::PowerOffAnimation(LedMatrix& m)
	: OverlayAnimation(m, POWEROFF_DEFAULT_HUE, POWEROFF_DEFAULT_SAT, POWEROFF_DEFAULT_VAL) {
	// пусто
}

void PowerOffAnimation::setProgress(uint8_t p) {
	progress = p;
}



void PowerOffAnimation::render() {
	matrix.clear();

	int w = matrix.getWidth();
	int h = matrix.getHeight();
	if (w <= 0) w = 1;
	if (h <= 0) h = 1;

	// С учётом XY-мэппинга (логический верх = физический низ),
	// чтобы физически гасить сверху вниз, необходимо логически гасить снизу вверх.
	// Инвертируем прогресс: 255 (начало удержания) = все горят;
	// по мере удержания гасим сверху вниз.
	uint32_t ext = (uint32_t)(255 - progress);
	uint32_t prod = ext * (uint32_t)h;               // 0..(h*255)
	int offRows = (int)(prod / 255);                 // полностью погашенные сверху (логически)
	int rem = (int)(prod % 255);                     // доля текущей гасимой строки

	for (int y = 0; y < h; ++y) {
		uint8_t rowV;
		if (y < offRows) {
			rowV = 0; // верхние (логически) уже погасли → физически верхние
		} else if (y == offRows) {
			// текущая строка гаснет плавно
			rowV = (uint8_t)((uint32_t)(255 - rem) * (uint32_t)val / 255U);
		} else {
			rowV = val; // остальные ещё горят
		}

		for (int x = 0; x < w; ++x) {
			if (rowV > 0) matrix.setPixelHSV(x, y, POWEROFF_DEFAULT_HUE, POWEROFF_DEFAULT_SAT, rowV);
		}
	}

}
