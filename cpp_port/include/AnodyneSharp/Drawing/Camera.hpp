#pragma once
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Utilities/MathUtilities.hpp"

namespace AnodyneSharp::Drawing {

class Camera {
public:
    Matrix View;
    Matrix Transform;
    Vector3 Offset = {0,0,0};
    float Zoom = 1.f;

    Vector3 Position() const {
        return {(float)(int)actual_pos.X, (float)(int)actual_pos.Y, actual_pos.Z};
    }

    Vector2 Position2D() const {
        auto pos = Position();
        return Vector2{pos.X, pos.Y} - Vector2{80.f, 80.f};
    }

    Rectangle Bounds() const {
        auto pos = Position2D();
        return {(int)pos.X, (int)pos.Y, 160, 160};
    }

    Camera() : actual_pos{0,0,0} {}
    Camera(Vector2 position) : actual_pos{position.X, position.Y, 0} {}

    void Reset() { Offset={0,0,0}; Zoom=1.f; actual_pos={0,0,0}; }

    void Recalc() {
        auto pos = Position();
        View = Matrix::CreateLookAt(pos, {pos.X, pos.Y, -1.f}, {0,1,0});
        Transform =
            Matrix::CreateTranslation({-pos.X,-pos.Y,-pos.Z}) *
            Matrix::CreateScale({Zoom, Zoom, 1.f}) *
            Matrix::CreateTranslation({80.f, 80.f, 0.f}) *
            Matrix::CreateTranslation(Offset);
    }

    bool GoTowards(Vector2 target, float speed) {
        bool x = MathUtilities::MoveTo(actual_pos.X, target.X + 80.f, speed);
        bool y = MathUtilities::MoveTo(actual_pos.Y, target.Y + 80.f, speed);
        Recalc();
        return x && y;
    }

    void GoTo(Vector2 target) {
        actual_pos = {target.X + 80.f, target.Y + 80.f, actual_pos.Z};
        Recalc();
    }
    void GoTo(float x, float y) { GoTo({x, y}); }

private:
    Vector3 actual_pos = {0,0,0};
};

} // namespace AnodyneSharp::Drawing

using AnodyneSharp::Drawing::Camera;
