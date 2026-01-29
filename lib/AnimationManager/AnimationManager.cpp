#include "AnimationManager.hpp"

void AnimationManager::addAnimation(AnimationBase* a) {
    if (!a) return;
    animations.push_back(a);
    if (animations.size() == 1) currentIndex = 0;
}

void AnimationManager::switchToNext() {
    if (animations.empty()) return;
    currentIndex = (currentIndex + 1) % static_cast<int>(animations.size());
}

void AnimationManager::switchToPrevious() {
    if (animations.empty()) return;
    int n = static_cast<int>(animations.size());
    currentIndex = (currentIndex - 1);
    if (currentIndex < 0) currentIndex += n;
}

bool AnimationManager::setAnimation(uint16_t id) {
    if (animations.empty()) return false;
    for (size_t i = 0; i < animations.size(); ++i) {
        if (animations[i] && animations[i]->getId() == id) {
            currentIndex = static_cast<int>(i);
            return true;
        }
    }
    return false;
}

AnimationBase* AnimationManager::getCurrentAnimation() const {
    if (animations.empty() || currentIndex < 0 || currentIndex >= static_cast<int>(animations.size())) return nullptr;
    return animations[currentIndex];
}

void AnimationManager::setOverlay(OverlayAnimation* ov) {
    overlay = ov;
}

void AnimationManager::unsetOverlay() {
    overlay = nullptr;
}

void AnimationManager::render() {
    if (!matrix) return;
    if (overlay) {
        overlay->render();
        return;
    }
    AnimationBase* cur = getCurrentAnimation();
    if (cur) {
        cur->render(*matrix);
    }
}
