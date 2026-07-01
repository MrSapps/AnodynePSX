#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp {

class Anim {
public:
    std::string name;
    std::vector<int> frames;
    bool looped = true;
    bool Dirty = true;

    float FrameRate() const { return delay == 0.f ? 0.f : 1.0f / delay; }
    void  SetFrameRate(float value) { delay = value == 0.f ? 0.f : 1.0f / value; }

    int Frame()    const { return frames[_curIndex]; }
    int CurIndex() const { return _curIndex; }
    bool Finished()const { return delay == 0.f || (!looped && _finishedLastFrame); }

    Anim() = default;
    Anim(const std::string& name, std::vector<int> frames, float frameRate, bool looped = true)
        : name(name), frames(std::move(frames)), looped(looped), Dirty(true) {
        SetFrameRate(frameRate);
    }

    void Reset() {
        _curIndex = 0;
        _frameTimer = 0.f;
        _finishedLastFrame = false;
        Dirty = true;
    }

    void Update();

private:
    float delay = 0.f;
    bool  _finishedLastFrame = false;
    int   _curIndex = 0;
    float _frameTimer = 0.f;
};

} // namespace AnodyneSharp

using AnodyneSharp::Anim;
