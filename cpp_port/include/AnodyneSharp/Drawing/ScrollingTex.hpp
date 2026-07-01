#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp::Drawing {

// Scrolling texture — used for parallax backgrounds, water, etc.
class ScrollingTex {
public:
    ScrollingTex() = default;

    // tex: texture name, scroll speed per second in pixels
    void Load(const std::string& texName, float scrollX, float scrollY);
    void Update();
    void Draw(float z = 0.5f);

    bool visible = true;
    Vector2 Position;
    Color   Tint = Color::White;

private:
    std::string _texName;
    Texture2D*  _tex     = nullptr;
    float       _scrollX = 0.f;
    float       _scrollY = 0.f;
    float       _offsetX = 0.f;
    float       _offsetY = 0.f;
    int         _w = 0, _h = 0;
};

} // namespace AnodyneSharp::Drawing
