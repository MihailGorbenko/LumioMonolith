#pragma once
#include <vector>
#include "../Animation/Animation.hpp"
#include "../Animation/OverlayAnimation.hpp"
#include "AnimMngrCfg.hpp"
#include "../StorageManager/StorageManager.hpp"

// Autosave timeout for AnimationManager (milliseconds)
#ifndef ANIM_MNGR_AUTOSAVE_MS
#define ANIM_MNGR_AUTOSAVE_MS 60000UL
#endif

// Number of autosave retry attempts before giving up
#ifndef ANIM_MNGR_AUTOSAVE_RETRIES
#define ANIM_MNGR_AUTOSAVE_RETRIES 3
#endif

class AnimationManager {
public:
    explicit AnimationManager(StorageManager& s) : storage(s) {}

    // Add animation to the manager (does not take ownership)
    void addAnimation(AnimationBase* a);

    // Cycle to next animation in the list (wrap around)
    void switchToNext();

    // Cycle to previous animation in the list (wrap around)
    void switchToPrevious();

    // Select animation by its stable ID; keeps index consistent with switchToNext
    bool setAnimation(uint16_t id);

    
    void init();

    // Overlay control: set/unset system overlay animation
    void setOverlay(OverlayAnimation* ov);
    void unsetOverlay();

    // Render current frame: overlay if set, otherwise base animation
    void render();

    // Periodic update: perform autosave and persistence tasks
    void update();

    // Force persist manager config and dirty animations
    bool forceSave();
    // Set hue for the currently active animation (0..255)
    bool setCurrentHue(uint8_t hue);

    // Get hue for the currently active animation (0..255)
    uint8_t getCurrentHue() const;
    // Get current animation id (0 if none)
    uint16_t getCurrentId() const;

    // Get current animation name (returns nullptr if none)
    const char* getCurrentName() const;

private:
    StorageManager& storage;
    bool configDirty{false};
    // persisted manager config
    AnimMngrCfg animCfg;
    std::vector<AnimationBase*> animations;
    int currentIndex{0};
    OverlayAnimation* overlay{nullptr};
    // Timing / dirty tracking for autosave logic
    unsigned long lastDirtyMs{0};
    bool anyAnimDirty{false};
    int dirtySaveAttempts{0};

    // Helpers for tracking manager config dirty state
    void setConfigDirty();
    void clearConfigDirty();
    // Load configs for current animation and neighbors
    void loadNeighbors();
};
