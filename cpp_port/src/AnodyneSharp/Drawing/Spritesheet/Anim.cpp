#include "AnodyneSharp/Drawing/Spritesheet/Anim.hpp"
#include "AnodyneSharp/GameTimes.hpp"

namespace AnodyneSharp {

void Anim::Update() {
    if (Finished()) return;
    _frameTimer += GameTimes::DeltaTime();
    while (_frameTimer > delay) {
        _frameTimer -= delay;
        if (_curIndex == (int)frames.size() - 1) {
            _finishedLastFrame = true;
            if (looped) _curIndex = 0;
        } else {
            _curIndex++;
        }
        Dirty = true;
    }
}

} // namespace AnodyneSharp
