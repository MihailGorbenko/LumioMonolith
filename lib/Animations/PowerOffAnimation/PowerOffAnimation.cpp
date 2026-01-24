#include "PowerOffAnimation.hpp"

PowerOffAnimation::PowerOffAnimation(LedMatrix& m)
	: matrix(&m),
	  hue(POWEROFF_DEFAULT_HUE),
	  sat(POWEROFF_DEFAULT_SAT),
	  val(POWEROFF_DEFAULT_VAL),
	  progress(0) {
	// пусто
}

void PowerOffAnimation::setProgress(uint8_t p) {
	progress = p;
}

void PowerOffAnimation::setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
	// заглушка: обновляем цвет, но не сохраняем
	hue = h; sat = s; val = v;
}

bool PowerOffAnimation::saveColor(const char* key) {
	Serial.print("PowerOffAnimation: saveColor - stub, not saved for key: ");
	Serial.println(key ? key : "(null)");
	return false;
}

bool PowerOffAnimation::loadColor(const char* key) {
	Serial.print("PowerOffAnimation: loadColor - stub, not loaded for key: ");
	Serial.println(key ? key : "(null)");
	return false;
}

void PowerOffAnimation::render() {
	if (!matrix) return;
	matrix->clear();

	int w = matrix->width();
	int h = matrix->height();
	if (w <= 0) w = 1;
	if (h <= 0) h = 1;

	// Затухание сегментов сверху вниз:
	// при удержании прогресс уменьшается 255 -> 0,
	// полностью гасим строки сверху, последние остаются снизу.
	uint32_t prod = (uint32_t)progress * (uint32_t)h; // 0..(h*255)
	int filledRows = (int)(prod / 255);               // полные снизу
	int rem = (int)(prod % 255);                      // частично заполненная строка снизу

	for (int y = 0; y < h; ++y) {
		uint8_t rowV = 0;
		int bottomStart = h - filledRows; // индекс первой полной снизу
		if (y >= bottomStart) {
			rowV = val; // полная яркость у нижних строк
		} else if (rem > 0 && y == (bottomStart - 1)) {
			// частично заполненная строка над полными
			rowV = (uint8_t)((uint32_t)rem * (uint32_t)val / 255U);
		} else {
			rowV = 0; // верхние уже погасли
		}

		for (int x = 0; x < w; ++x) {
			if (rowV > 0) {
				matrix->setPixelHSV(x, y, hue, sat, rowV);
			}
		}
	}

	matrix->show();
}

