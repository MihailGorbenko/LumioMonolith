#ifndef ANIMATION_HPP
#define ANIMATION_HPP
#include <Arduino.h>
#include "../StorageManager/Serializable.hpp"
#include "../LedMatrix/LedMatrix.hpp"

// Master default value for all animations (master brightness controls final brightness)
#ifndef ANIMATION_DEFAULT_VAL
#define ANIMATION_DEFAULT_VAL 255
#endif

class AnimationBase {
protected:
	LedMatrix* matrix;
	uint8_t hue;
	uint8_t sat;
	uint8_t val;
	uint8_t defaultHue;
	uint8_t defaultSat;
	uint8_t defaultVal;
	const char* name;
public:
	// принимает ссылку на LedMatrix и defaultColor (h,s,v)
	    explicit AnimationBase(LedMatrix& m, uint8_t defH = 0, uint8_t defS = 255, uint8_t defV = 255)
		: matrix(&m),
		  hue(defH), sat(defS), val(defV),
		    defaultHue(defH), defaultSat(defS), defaultVal(defV), name("Unnamed") {}
	virtual ~AnimationBase() {}

	// установить цвет в HSV (0..255)
	virtual void setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
		hue = h; sat = s; val = v;
	}

	// установить/получить читаемое имя анимации
	const char* getName() const { return name ? name : "Unnamed"; }

	// вызывается контроллером при активации анимации (переключение/включение)
	virtual void onActivate() {}

	// наследники реализуют логику анимации в update()
	virtual void render() = 0;


	// Опционально: сериализация через интерфейс (по умолчанию не поддерживается)
	virtual ISerializable* serializable() { return nullptr; }
};

#endif // ANIMATION_HPP
