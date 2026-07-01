#include "AnodyneSharp/Utilities/MathUtilities.hpp"
#define _USE_MATH_DEFINES
#include <cmath>

namespace AnodyneSharp {

bool MathUtilities::MoveTo(float& v, float target, float speed) {
    if (std::abs(target - v) <= speed) { v = target; return true; }
    v += (target > v ? 1.f : -1.f) * speed;
    return false;
}

void MathUtilities::RotateAround(const Vector2& Center, Vector2& rotator,
                                 float rotation_speed, float radius) {
    float angle = std::atan2(rotator.Y - Center.Y, rotator.X - Center.X);
    angle += rotation_speed;
    rotator.X = Center.X + std::cos(angle) * radius;
    rotator.Y = Center.Y + std::sin(angle) * radius;
}

int MathUtilities::OneRandomOf(std::initializer_list<int> p) {
    std::vector<int> v(p);
    if (v.empty()) return 0;
    int total = 0;
    for (int x : v) total += x;
    if (total <= 0) return -1;
    std::uniform_int_distribution<int> d(0, total - 1);
    // Use a static mt19937 since GlobalState isn't available here
    static std::mt19937 rng(std::random_device{}());
    int r = d(rng);
    int acc = 0;
    for (int i = 0; i < (int)v.size(); ++i) {
        acc += v[i];
        if (r < acc) return i;
    }
    return (int)v.size() - 1;
}

} // namespace AnodyneSharp
