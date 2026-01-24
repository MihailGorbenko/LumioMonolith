#ifndef ANIMATION_HPP
#define ANIMATION_HPP
#include <Arduino.h>
#include "../Persistant/Persistant.hpp"
#include "../LedMatrix/LedMatrix.hpp"

// Master default value for all animations (master brightness controls final brightness)
#ifndef ANIMATION_DEFAULT_VAL
#define ANIMATION_DEFAULT_VAL 255
#endif

class AnimationBase : public IPersistant {
protected:
	LedMatrix* matrix;
	uint8_t hue;
	uint8_t sat;
	uint8_t val;
	uint8_t defaultHue;
	uint8_t defaultSat;
	uint8_t defaultVal;
public:
	// принимает ссылку на LedMatrix и defaultColor (h,s,v)
	explicit AnimationBase(LedMatrix& m, uint8_t defH = 0, uint8_t defS = 255, uint8_t defV = 255)
		: matrix(&m),
		  hue(defH), sat(defS), val(defV),
		  defaultHue(defH), defaultSat(defS), defaultVal(defV) {}
	virtual ~AnimationBase() {}

	// установить цвет в HSV (0..255)
	virtual void setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
		hue = h; sat = s; val = v;
	}

	// вызывается контроллером при активации анимации (переключение/включение)
	virtual void onActivate() {}

	// наследники реализуют логику анимации в update()
	virtual void render() = 0;

	// соответствие интерфейсу IPersistant: loadFromNVS
	bool saveToNVS(const char* key) override;
	bool loadFromNVS(const char* key) override;
};

#endif // ANIMATION_HPP
