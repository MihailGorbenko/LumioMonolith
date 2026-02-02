#include "AnimationManager.hpp"
#include "../StorageManager/StorageManager.hpp"
#include <cstring>

void AnimationManager::addAnimation(AnimationBase* a) {
    if (!a) return;
    animations.push_back(a);
    if (animations.size() == 1) currentIndex = 0;
}

void AnimationManager::init(StorageManager& s) {
    storage = &s;
    // attempt to load persisted manager config
    if (storage) {
        // try to load stored manager config
        bool ok = storage->loadAnimMngrCfg(animCfg);
        LOGF("AnimMngr", "init loadAnimMngrCfg ok=%d lastAnimId=%u\n", (int)ok, (unsigned)animCfg.lastAnimId);
        // find loaded id in animations list; fallback to first animation when missing/failure
        int chosen = 0;
        if (ok && animCfg.lastAnimId != 0) {
            for (size_t i = 0; i < animations.size(); ++i) {
                if (animations[i] && animations[i]->getId() == animCfg.lastAnimId) { chosen = (int)i; break; }
            }
        } else {
            // default to first available animation
            if (!animations.empty()) {
                animCfg.lastAnimId = animations[0]->getId();
            } else {
                animCfg.lastAnimId = 0;
            }
        }
        // set current index to chosen
        if (!animations.empty()) currentIndex = chosen;

        // activate current animation
        if (!animations.empty()) {
            AnimationBase* cur = animations[currentIndex];
            if (cur && !cur->isInitialized()) {
                // load its saved config if available
                if (storage) storage->loadAnimation(*cur);
                LOGF("AnimMngr", "init loaded anim id=%u initialized=%d\n", (unsigned)cur->getId(), (int)cur->isInitialized());
                cur->onActivate();
            } else if (cur) {
                LOGF("AnimMngr", "init activating anim id=%u alreadyInitialized=%d\n", (unsigned)cur->getId(), (int)cur->isInitialized());
                cur->onActivate();
            }
            // preload and activate neighbors
            loadNeithboors();
        }
        // record timestamps
        lastSwitchMs = millis();
        lastHueChangeMs = millis();
    }
}

void AnimationManager::switchToNext() {
    if (animations.empty()) return;
    int n = (int)animations.size();
    int newIdx = (currentIndex + 1) % n;
    // handle activation/loading of target if needed
    AnimationBase* target = animations[newIdx];
    if (target) {
        uint16_t prevId = (animations[currentIndex] ? animations[currentIndex]->getId() : 0);
        LOGF("AnimMngr", "switchToNext %d->%d prevId=%u nextId=%u\n", currentIndex, newIdx, (unsigned)prevId, (unsigned)target->getId());
    }
    if (target && !target->isInitialized()) {
        if (storage) storage->loadAnimation(*target);
        target->onActivate();
    }
    // load/activate neighbors if needed
    currentIndex = newIdx;
    animCfg.lastAnimId = (animations[currentIndex] ? animations[currentIndex]->getId() : 0);
    setConfigDirty();
    loadNeithboors();
    lastSwitchMs = millis();
}

void AnimationManager::switchToPrevious() {
    if (animations.empty()) return;
    int n = (int)animations.size();
    int newIdx = currentIndex - 1;
    if (newIdx < 0) newIdx += n;
    AnimationBase* target = animations[newIdx];
    if (target) {
        uint16_t prevId = (animations[currentIndex] ? animations[currentIndex]->getId() : 0);
        LOGF("AnimMngr", "switchToPrevious %d->%d prevId=%u nextId=%u\n", currentIndex, newIdx, (unsigned)prevId, (unsigned)target->getId());
    }
    if (target && !target->isInitialized()) {
        if (storage) storage->loadAnimation(*target);
        target->onActivate();
    }
    currentIndex = newIdx;
    animCfg.lastAnimId = (animations[currentIndex] ? animations[currentIndex]->getId() : 0);
    setConfigDirty();
    loadNeithboors();
    lastSwitchMs = millis();
}

bool AnimationManager::setAnimation(uint16_t id) {
    if (animations.empty()) return false;
    for (size_t i = 0; i < animations.size(); ++i) {
        if (animations[i] && animations[i]->getId() == id) {
            int newIdx = (int)i;
            AnimationBase* target = animations[newIdx];
            LOGF("AnimMngr", "setAnimation id=%u idx=%d\n", (unsigned)id, newIdx);
            if (target && !target->isInitialized()) {
                if (storage) storage->loadAnimation(*target);
                target->onActivate();
            } else if (target) {
                target->onActivate();
            }
            currentIndex = newIdx;
            animCfg.lastAnimId = id;
            setConfigDirty();
            loadNeithboors();
            lastSwitchMs = millis();
            return true;
        }
    }
    return false;
}

void AnimationManager::setOverlay(OverlayAnimation* ov) {
    overlay = ov;
}

void AnimationManager::unsetOverlay() {
    overlay = nullptr;
}

void AnimationManager::render() {
    if (overlay) {
        overlay->render();
        return;
    }
    if (!animations.empty() && currentIndex >= 0 && currentIndex < (int)animations.size()) {
        AnimationBase* cur = animations[currentIndex];
        if (cur) cur->render();
    }
}

void AnimationManager::update() {
    // lightweight autosave checks (non-blocking)
    unsigned long now = millis();
    const unsigned long AUTOSAVE_MS = 60000UL;
    if (storage) {
        if (configDirty && (now - lastSwitchMs) >= AUTOSAVE_MS) {
            LOGF("AnimMngr", "autosave: manager config dirty, forcing save\n");
            forceSave();
            lastSwitchMs = now;
        }
        if ((now - lastHueChangeMs) >= AUTOSAVE_MS) {
            // save all dirty animation configs if any
            bool anyDirty = false;
            for (auto a : animations) { if (a && a->isConfigDirty()) { anyDirty = true; break; } }
            if (anyDirty) {
                // save each dirty animation
                for (auto a : animations) {
                    if (a && a->isConfigDirty()) {
                        bool ok = storage->saveAnimation(*a);
                        LOGF("AnimMngr", "autosave anim id=%u ok=%d\n", (unsigned)a->getId(), (int)ok);
                        if (ok) a->clearConfigDirty();
                    }
                }
                lastHueChangeMs = now;
            }
        }
    }
}

bool AnimationManager::isConfigDirty() const {
    return configDirty;
}

void AnimationManager::setConfigDirty() {
    configDirty = true;
}

void AnimationManager::clearConfigDirty() {
    configDirty = false;
}

bool AnimationManager::forceSave() {
    if (!storage) return false;
    bool okAll = true;
    if (configDirty) {
        bool ok = storage->saveAnimMngrCfg(animCfg);
        if (ok) clearConfigDirty();
        okAll = okAll && ok;
        LOGF("AnimMngr", "forceSave manager ok=%d lastAnimId=%u\n", (int)ok, (unsigned)animCfg.lastAnimId);
    }
    // Save dirty animation configs
    for (auto a : animations) {
        if (a && a->isConfigDirty()) {
            bool ok = storage->saveAnimation(*a);
            LOGF("AnimMngr", "forceSave anim id=%u ok=%d\n", (unsigned)a->getId(), (int)ok);
            if (ok) a->clearConfigDirty();
            okAll = okAll && ok;
        }
    }
    return okAll;
}

void AnimationManager::loadNeithboors() {
    if (!storage) return;
    int n = (int)animations.size();
    if (n <= 0) return;
    int idx = currentIndex;
    auto tryLoad = [&](int i) {
        if (i < 0 || i >= n) return;
        AnimationBase* a = animations[i];
        if (!a) return;
        if (!a->isInitialized()) {
            bool ok = storage->loadAnimation(*a);
            LOGF("AnimMngr", "loadNeithboors load id=%u ok=%d\n", (unsigned)a->getId(), (int)ok);
            a->onActivate();
            a->clearConfigDirty();
        }
    };
    tryLoad(idx);
    tryLoad(idx - 1);
    tryLoad(idx + 1);
}

bool AnimationManager::setCurrentHue(uint8_t hue) {
    if (animations.empty() || currentIndex < 0 || currentIndex >= (int)animations.size()) return false;
    AnimationBase* cur = animations[currentIndex];
    if (!cur) return false;
    cur->setHue(hue);
    // mark animation config dirty and record last hue change timestamp
    cur->setConfigDirty();
    lastHueChangeMs = millis();
    // Per requirements, do not save immediately here — saving happens on timer or via forceSave()
    return true;
}

// getCurrentHue removed

bool AnimationManager::saveCurrentAnimationIfDirty() {
    if (!storage) return false;
    if (animations.empty() || currentIndex < 0 || currentIndex >= (int)animations.size()) return false;
    AnimationBase* cur = animations[currentIndex];
    if (!cur) return false;
    if (cur->isConfigDirty()) {
        bool ok = storage->saveAnimation(*cur);
        if (ok) cur->clearConfigDirty();
        return ok;
    }
    return true;
}

uint16_t AnimationManager::getCurrentId() const {
    if (animations.empty() || currentIndex < 0 || currentIndex >= (int)animations.size()) return 0;
    AnimationBase* cur = animations[currentIndex];
    return cur ? cur->getId() : 0;
}

const char* AnimationManager::getCurrentName() const {
    if (animations.empty() || currentIndex < 0 || currentIndex >= (int)animations.size()) return nullptr;
    AnimationBase* cur = animations[currentIndex];
    return cur ? cur->getName() : nullptr;
}
