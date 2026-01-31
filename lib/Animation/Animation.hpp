#ifndef ANIMATION_HPP
#define ANIMATION_HPP
#include <Arduino.h>
#include <cstring>
#include "AnimConfig.hpp"
class LedMatrix; // forward declaration to avoid heavy include dependency

// Значение мастер-яркости для всех анимаций (финальный множитель яркости)
#ifndef ANIMATION_DEFAULT_VAL
#define ANIMATION_DEFAULT_VAL 255
#endif

// Глобальная насыщенность по умолчанию (не хранится в конфиге анимации)
#ifndef ANIMATION_DEFAULT_SAT
#define ANIMATION_DEFAULT_SAT 255
#endif

// Базовый класс анимации: хранит конфиг цвета (только оттенок); матрица передаётся в render
class AnimationBase {
protected:
	AnimConfig animCfg;
	uint16_t animId;
	bool configDirty;

public:
	// Принимает оттенок по умолчанию и идентификатор анимации (матрица передаётся в render)
	explicit AnimationBase(uint8_t defH = 0, uint16_t id = 0);

	// Установить оттенок (0..255) — частый вызов, без virtual
	inline void setHue(uint8_t h) {
		if (h != animCfg.hue) {
			animCfg.setHue(h);
			configDirty = true;
		}
	}
	inline bool isConfigDirty() const { return configDirty; }
	inline void clearConfigDirty() { configDirty = false; }

	// Доступ к конфигурации (хранит только оттенок)
	inline const AnimConfig& getConfig() const { return animCfg; }
	inline AnimConfig& getConfig() { return animCfg; }

	// Получить ID анимации
	inline uint16_t getId() const { return animId; }

	// Имя анимации определяется в подклассах (базовый дефолт)
	virtual const char* getName() const = 0;

	// Формирует краткий ключ NVS по ID (формат: "a<id>")
	// Гарантированно укладывается в лимит NVS (<=15 символов),
	// записывает NUL-терминированную строку в out.
	void makeNvsKeyById(char* out, size_t outSize) const;

	// Наследники реализуют логику анимации; матрица передаётся параметром
	virtual void render(LedMatrix& m) = 0;

};
#endif // ANIMATION_HPP
