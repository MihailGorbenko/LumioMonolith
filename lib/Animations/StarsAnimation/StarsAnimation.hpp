#ifndef STARS_ANIMATION_HPP
#define STARS_ANIMATION_HPP

#include <Arduino.h>
#include <vector>
#include "../../Animation/Animation.hpp"
#include "../../StorageManager/Serializable.hpp"

// default configuration (can be overridden in project before include)
#ifndef STARS_DEFAULT_HUE
#define STARS_DEFAULT_HUE 0
#endif
#ifndef STARS_DEFAULT_SAT
#define STARS_DEFAULT_SAT 255
#endif
#ifndef STARS_DEFAULT_VAL
#define STARS_DEFAULT_VAL ANIMATION_DEFAULT_VAL
#endif
#ifndef STARS_STAR_COUNT
#define STARS_STAR_COUNT 20
#endif

// Readable name
#ifndef STARS_ANIMATION_NAME
#define STARS_ANIMATION_NAME "Stars"
#endif


class StarsAnimation : public AnimationBase {
public:
	// default hue comes from defines; id and matrix are provided by main
	explicit StarsAnimation(uint16_t id, LedMatrix& m);

	// render one frame
	void render() override;

	// activation hook to perform expensive preparation
	void onActivate() override;

	const char* getName() const override { return STARS_ANIMATION_NAME; }

private:
	struct Star {
		uint8_t x;
		uint8_t y;
		uint8_t brightness;    // current brightness value (0..255)
		uint8_t target;        // target brightness
		unsigned long nextChangeMillis;
		// Parallax: depth layer and sub-pixel X position/velocity
		uint8_t depth;          // 0=far (slow), 1=mid, 2=near (fast)
		int32_t xfp;            // fixed-point X (8.8), x = xfp >> 8
		int8_t vfp;             // X velocity in fixed-point (px*256/frame)
		// Smooth twinkle: phase and speed of sinusoidal modulation
		uint8_t twPhase;        // 0..255
		uint8_t twSpeed;        // small values for slow twinkle
	};

	std::vector<Star> stars;
	int starCount;
	// timestamp of last frame for dt-dependent motion
	uint32_t lastMillis = 0;
    
    // cached matrix size populated in onActivate()
    int cachedWidth = 0;
    int cachedHeight = 0;


	// helpers
	void randomizeStar(Star& s, int w, int h);
};

#endif // STARS_ANIMATION_HPP
