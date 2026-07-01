#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp {

// Mirrors C# GameTimes static class
class GameTimes {
public:
    static float FPS;
    static float TimeScale;
    static float TrueDeltaTime;

    static float DeltaTime() {
        return TrueDeltaTime * TimeScale;
    }

    static void UpdateTimes(const GameTime& gameTime) {
        TrueDeltaTime = (float)gameTime.ElapsedGameTime.TotalSeconds();
    }

    static void UpdateFPS(const GameTime& gameTime) {
        if (_fpsQueue.size() > (size_t)_maxSamples) {
            _fpsQueue.pop();
            float sum = 0.f;
            auto copy = _fpsQueue;
            while (!copy.empty()) { sum += copy.front(); copy.pop(); }
            FPS = sum / _fpsQueue.size();
        }
        float fps = 1.f / (float)gameTime.ElapsedGameTime.TotalSeconds();
        _fpsQueue.push(fps);
    }

private:
    static std::queue<float> _fpsQueue;
    static int _maxSamples;
};

} // namespace AnodyneSharp

using AnodyneSharp::GameTimes;
