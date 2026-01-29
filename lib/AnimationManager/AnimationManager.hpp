#pragma once
#include <vector>
#include "../LedMatrix/LedMatrix.hpp"
#include "../Animation/Animation.hpp"
#include "../Animation/OverlayAnimation.hpp"

class AnimationManager {
public:
    explicit AnimationManager(LedMatrix& m) : matrix(&m) {}

    // Add animation to the manager (does not take ownership)
    void addAnimation(AnimationBase* a);

    // Cycle to next animation in the list (wrap around)
    void switchToNext();

    // Cycle to previous animation in the list (wrap around)
    void switchToPrevious();

    // Select animation by its stable ID; keeps index consistent with switchToNext
    bool setAnimation(uint16_t id);

    // Get pointer to currently selected animation (nullptr if none)
    AnimationBase* getCurrentAnimation() const;

    // Overlay control: set/unset system overlay animation
    void setOverlay(OverlayAnimation* ov);
    void unsetOverlay();

    // Render current frame: overlay if set, otherwise base animation
    void render();

private:
    LedMatrix* matrix;
    std::vector<AnimationBase*> animations;
    int currentIndex{0};
    OverlayAnimation* overlay{nullptr};
};
