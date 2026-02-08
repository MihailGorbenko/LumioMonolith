#include "AnimationManager.hpp"
#include "../StorageManager/StorageManager.hpp"
#include "../../src/debug.hpp"

void AnimationManager::addAnimation(AnimationBase* a) {
    if (!a) return;
    animations.push_back(a);
    if (animations.size() == 1) currentIndex = 0;
}

void AnimationManager::init() {
    // attempt to load persisted manager config
    bool ok = storage.loadAnimMngrCfg(animCfg);
    LOGF("AnimMngr", "init loadAnimMngrCfg ok=%d lastAnimId=%u\n", (int)ok, (unsigned)animCfg.lastAnimId);

    // find desired id in animations list; fallback to first animation when missing/failure
    int chosen = 0;
    uint16_t desiredId = 1; // default id when no stored value
    if (ok && animCfg.lastAnimId != 0) desiredId = animCfg.lastAnimId;
    bool found = false;
    for (size_t i = 0; i < animations.size(); ++i) {
        if (animations[i] && animations[i]->getId() == desiredId) { chosen = (int)i; found = true; break; }
    }
    if (found) {
        animCfg.lastAnimId = desiredId;
    } else {
        if (!animations.empty()) {
            chosen = 0;
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
            // load its saved config if available and activate once
            storage.loadAnimation(*cur);
            LOGF("AnimMngr", "init loaded anim id=%u initialized=%d\n", (unsigned)cur->getId(), (int)cur->isInitialized());
            cur->onActivate();
        } else if (cur) {
            LOGF("AnimMngr", "init anim id=%u alreadyInitialized=%d (skipping onActivate)\n", (unsigned)cur->getId(), (int)cur->isInitialized());
        }
        // preload and activate neighbors
        loadNeighbors();
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
        storage.loadAnimation(*target);
        target->onActivate();
    }
    // load/activate neighbors if needed
    currentIndex = newIdx;
    animCfg.lastAnimId = (animations[currentIndex] ? animations[currentIndex]->getId() : 0);
    setConfigDirty();
    loadNeighbors();
    // record dirty-timer for manager config change (handled in setConfigDirty)
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
        storage.loadAnimation(*target);
        target->onActivate();
    }
    currentIndex = newIdx;
    animCfg.lastAnimId = (animations[currentIndex] ? animations[currentIndex]->getId() : 0);
    setConfigDirty();
    loadNeighbors();
    // record dirty-timer for manager config change (handled in setConfigDirty)
}

bool AnimationManager::setAnimation(uint16_t id) {
    if (animations.empty()) return false;
    for (size_t i = 0; i < animations.size(); ++i) {
        if (animations[i] && animations[i]->getId() == id) {
            int newIdx = (int)i;
            AnimationBase* target = animations[newIdx];
            LOGF("AnimMngr", "setAnimation id=%u idx=%d\n", (unsigned)id, newIdx);
            if (target && !target->isInitialized()) {
                storage.loadAnimation(*target);
                target->onActivate();
            }
            currentIndex = newIdx;
            animCfg.lastAnimId = id;
            setConfigDirty();
            loadNeighbors();
            // record dirty-timer for manager config change (handled in setConfigDirty)
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
    // If either manager config or any animation config is dirty and the dirty timer expired, force save
    if ((configDirty || anyAnimDirty) && lastDirtyMs != 0 && (now - lastDirtyMs) >= ANIM_MNGR_AUTOSAVE_MS) {
        LOGF("AnimMngr", "autosave: dirty flags set, forcing save\n");
        bool ok = forceSave();
        // If save succeeded, clear dirty timer and attempts. If it failed, schedule retry.
        if (ok) {
            lastDirtyMs = 0;
            dirtySaveAttempts = 0;
        } else {
            // increment attempts and either schedule next retry or give up
            dirtySaveAttempts++;
            if (dirtySaveAttempts >= ANIM_MNGR_AUTOSAVE_RETRIES) {
                LOGF("AnimMngr", "autosave: giving up after %d attempts\n", dirtySaveAttempts);
                // stop auto-retries until a new change occurs
                lastDirtyMs = 0;
                dirtySaveAttempts = 0;
            } else {
                // reset timer to now so next attempt happens after full timeout
                lastDirtyMs = now;
            }
        }
    }
}

void AnimationManager::setConfigDirty() {
    configDirty = true;
    lastDirtyMs = millis();
    dirtySaveAttempts = 0;
}

void AnimationManager::clearConfigDirty() {
    configDirty = false;
}

bool AnimationManager::forceSave() {
    bool okAll = true;
    // Save manager config if dirty
    if (configDirty) {
        bool ok = storage.saveAnimMngrCfg(animCfg);
        LOGF("AnimMngr", "forceSave manager ok=%d lastAnimId=%u\n", (int)ok, (unsigned)animCfg.lastAnimId);
        if (ok) clearConfigDirty();
        okAll = okAll && ok;
    }
    // Save dirty animations if flagged
    if (anyAnimDirty) {
        bool anyFailed = false;
        for (auto a : animations) {
            if (a && a->isConfigDirty()) {
                bool ok = storage.saveAnimation(*a);
                LOGF("AnimMngr", "forceSave anim id=%u ok=%d\n", (unsigned)a->getId(), (int)ok);
                if (ok) a->clearConfigDirty();
                else anyFailed = true;
                okAll = okAll && ok;
            }
        }
        if (!anyFailed) anyAnimDirty = false;
    }
    if (!okAll) LOGF("AnimMngr", "forceSave: some saves failed (okAll=%d)\n", (int)okAll);
    return okAll;
}

void AnimationManager::loadNeighbors() {
    // storage is a required reference; attempt to load neighbors with wrap-around
    int n = (int)animations.size();
    if (n <= 0) return;
    int idx = currentIndex;
    if (idx < 0) idx = 0;
    if (idx >= n) idx = n - 1;
    auto tryLoadAt = [&](int i) {
        AnimationBase* a = animations[i];
        if (!a) return;
        if (!a->isInitialized()) {
            bool ok = storage.loadAnimation(*a);
            LOGF("AnimMngr", "loadNeighbors load id=%u ok=%d\n", (unsigned)a->getId(), (int)ok);
            a->onActivate();
            a->clearConfigDirty();
        }
    };
    // load current
    tryLoadAt(idx);
    // load previous with wrap-around
    int prev = (idx - 1 + n) % n;
    tryLoadAt(prev);
    // load next with wrap-around
    int next = (idx + 1) % n;
    tryLoadAt(next);
}

bool AnimationManager::setCurrentHue(uint8_t hue) {
    if (animations.empty() || currentIndex < 0 || currentIndex >= (int)animations.size()) return false;
    AnimationBase* cur = animations[currentIndex];
    if (!cur) return false;
    cur->setHue(hue);
    // mark animation config dirty and record last hue change timestamp
    cur->setConfigDirty();
    anyAnimDirty = true;
    lastDirtyMs = millis();
    dirtySaveAttempts = 0;
    // Per requirements, do not save immediately here — saving happens on timer or via forceSave()
    return true;
}

uint8_t AnimationManager::getCurrentHue() const {
    if (animations.empty() || currentIndex < 0 || currentIndex >= (int)animations.size()) return 0;
    AnimationBase* cur = animations[currentIndex];
    return cur ? cur->getConfig().hue : 0;
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
