#ifndef SCANNER_ANIMATION_HPP
#define SCANNER_ANIMATION_HPP

#include <Arduino.h>
#include "../../Animation/Animation.hpp"

#ifndef SCANNER_DEFAULT_HUE
#define SCANNER_DEFAULT_HUE 0
#endif
#ifndef SCANNER_DEFAULT_SAT
#define SCANNER_DEFAULT_SAT 255
#endif
#ifndef SCANNER_DEFAULT_VAL
#define SCANNER_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif

class ScannerAnimation : public AnimationBase {
public:
	explicit ScannerAnimation(LedMatrix& m);
	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) override;
	void render() override;

private:
	uint8_t tail; // 0..255, bigger = longer/brighter tail
};

#endif // SCANNER_ANIMATION_HPP
