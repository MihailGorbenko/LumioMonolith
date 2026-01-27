#ifndef ANIMATION_HPP
#define ANIMATION_HPP
#include <Arduino.h>
#include <cstring>
#include "../StorageManager/Serializable.hpp"
#include "../LedMatrix/LedMatrix.hpp"

// Master default value for all animations (master brightness controls final brightness)
#ifndef ANIMATION_DEFAULT_VAL
#define ANIMATION_DEFAULT_VAL 255
#endif

// Структура конфигурации анимации для оттенка и насыщенности
struct AnimCfg {
	uint8_t hue;
	uint8_t sat;
	AnimCfg(uint8_t h = 0, uint8_t s = 255) : hue(h), sat(s) {}
	inline void set(uint8_t h, uint8_t s) { hue = h; sat = s; }
};

static_assert(sizeof(AnimCfg) == 2, "AnimCfg layout changed");

class AnimationBase : public ISerializable {
protected:
	LedMatrix* matrix;

	// Конфигурации оттенка/насыщенности
	AnimCfg animCfg;
	AnimCfg defaultCfg;

public:
	// принимает ссылку на LedMatrix и defaultColor (h,s)
	explicit AnimationBase(LedMatrix& m, uint8_t defH = 0, uint8_t defS = 255)
		: matrix(&m), animCfg(defH, defS), defaultCfg(defH, defS) {}
	virtual ~AnimationBase() {}

	// установить цвет в HSV (0..255), v игнорируется (val всегда 255)
	virtual void setColorHSV(uint8_t h, uint8_t s) {
		animCfg.set(h, s);
	}


	// Имя анимации определяется в подклассах (базовый дефолт)
	virtual const char* getName() const { return "Unnamed"; }

	// Ключ NVS определяется в подклассах (базовый дефолт: нет ключа)
	virtual const char* getNvsKeyName() const { return nullptr; }

	// вызывается контроллером при активации анимации (переключение/включение)
	virtual void onActivate() {}

	// наследники реализуют логику анимации в render()
	virtual void render() = 0;

	// ISerializable: сериализация структуры AnimCfg (hue, sat)
	size_t serializedSize() const override {
		return sizeof(AnimCfg);
	}

	bool serialize(uint8_t* buf, size_t len) const override {
		if (!buf || len < sizeof(AnimCfg)) return false;
		memcpy(buf, &animCfg, sizeof(AnimCfg));
		return true;
	}

	bool deserialize(const uint8_t* buf, size_t len) override {
		if (!buf || len < sizeof(AnimCfg)) {
			// Ошибка буфера/длины — применяем дефолт
			setColorHSV(defaultCfg.hue, defaultCfg.sat);
			return false;
		}
		AnimCfg temp;
		memcpy(&temp, buf, sizeof(AnimCfg));
		// Валидация значений: насыщенность не должна быть 0 (полностью бесцветно)
		if (temp.sat == 0) {
			setColorHSV(defaultCfg.hue, defaultCfg.sat);
			return false;
		}
		setColorHSV(temp.hue, temp.sat);
		return true;
	}


};

#endif // ANIMATION_HPP
