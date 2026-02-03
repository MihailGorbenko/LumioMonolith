#include "Animation.hpp"
#include <cstring>
#include <cstdio>

AnimationBase::AnimationBase(uint8_t defH, uint16_t id, LedMatrix& m)
    : animCfg(defH), animId(id), configDirty(false), initialized(false), matrix(m) {}


void AnimationBase::makeNvsKeyById(char* out, size_t outSize) const {
    if (!out || outSize == 0) return;
    // Ensure first character is a letter per NVS constraints; prefix with 'a'
    // ID is uint16_t (0..65535), so decimal length <= 5 -> total <= 6
    // Guard against tiny buffers and ensure NUL termination
    if (outSize < 2) { out[0] = '\0'; return; }
    std::snprintf(out, outSize, "a%u", (unsigned)animId);
    out[outSize - 1] = '\0';
}
