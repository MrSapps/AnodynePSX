#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp {

class MathUtilities {
public:
    static Rectangle ScaleRectangle(const Rectangle& rect, float scale) {
        return CreateRectangle(rect.X * scale, rect.Y * scale,
                               rect.Width * scale, rect.Height * scale);
    }
    static Rectangle CreateRectangle(float x, float y, float width, float height) {
        return Rectangle{(int)x, (int)y, (int)width, (int)height};
    }
    static int OneRandomOf(std::initializer_list<int> p);

    static bool MoveTo(float& v, float target, float speed);

    static void RotateAround(const Vector2& Center, Vector2& rotator,
                             float rotation_speed, float radius);

    static std::string IntToString(int num) {
        switch (num) {
            case 1: return "one";   case 2: return "two";
            case 3: return "three"; case 4: return "four";
            case 5: return "five";  case 6: return "six";
            default: return ":(";
        }
    }
};

} // namespace AnodyneSharp

using AnodyneSharp::MathUtilities;
