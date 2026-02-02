#pragma once
#include <cstddef>
#include <Preferences.h>
#include "Serializable.hpp"

// Forward-declare to avoid cyclic include: full definition only needed in .cpp
class AnimationBase;

class StorageManager {
public:

    // App state (stored under namespace "app")
    bool saveApp(const ISerializable& obj);     // saves under key "cfg"
    bool loadApp(ISerializable& obj);           // loads from key "cfg"

    // Animation configs (stored under namespace "anim") via base class
    bool saveAnimation(AnimationBase& anim);
    bool loadAnimation(AnimationBase& anim);

    // Generic helpers (namespaced externally)
    bool saveSerializable(const char* ns, const char* key, const ISerializable& obj);
    bool loadSerializable(const char* ns, const char* key, ISerializable& obj);

    // Animation manager config (stored under namespace "anim_mngr")
    bool saveAnimMngrCfg(const ISerializable& obj); // saves under key "cfg"
    bool loadAnimMngrCfg(ISerializable& obj);       // loads from key "cfg"

private:
    Preferences prefs;
    static constexpr size_t SCRATCH_MAX = 64;
    uint8_t scratch[SCRATCH_MAX];
};
