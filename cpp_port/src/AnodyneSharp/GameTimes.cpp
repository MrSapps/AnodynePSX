#include "AnodyneSharp/GameTimes.hpp"

namespace AnodyneSharp {

float GameTimes::FPS            = 60.f;
float GameTimes::TimeScale      = 1.f;
float GameTimes::TrueDeltaTime  = 0.016f;
std::queue<float> GameTimes::_fpsQueue;
int   GameTimes::_maxSamples    = 60;

} // namespace AnodyneSharp
