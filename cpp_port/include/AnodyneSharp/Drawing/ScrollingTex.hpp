#pragma once
#include "AnodyneSharp/Common.hpp"
#include "DrawingUtilities.hpp"

namespace AnodyneSharp::Drawing {

// Scrolling texture — used for parallax backgrounds, water, etc.
class ScrollingTex {
public:
    ScrollingTex() = default;

    // tex: texture name, scroll speed per second in pixels
    void Load(const std::string& texName, Vector2 velocity, DrawOrder drawOrder);
    void Update();
    void DrawUI();
    void Draw();

    bool visible = true;
    Vector2 Position;
    Color   Tint = Color::White;

private:
    Texture2D*  _tex     = nullptr;
    Vector2     _velocity;
    DrawOrder   _layer = DrawOrder::BACKGROUND;
};

} // namespace AnodyneSharp::Drawing
