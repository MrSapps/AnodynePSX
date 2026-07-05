#include "AnodyneSharp/Drawing/ScrollingTex.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/GameTimes.hpp"

namespace AnodyneSharp::Drawing
{

    void ScrollingTex::Load(const std::string &texName, float scrollX, float scrollY)
    {
        SDL_Log("ScrollingTex::Load: loading texture '%s' with scroll speed (%.2f, %.2f)", texName.c_str(), scrollX, scrollY);
        _texName = texName;
        _scrollX = scrollX;
        _scrollY = -scrollY;
        _tex = Resources::ResourceManager::GetTexture(texName);
        if (_tex)
        {
            SDL_Log("ScrollingTex: Texture loaded");
            _w = _tex->Width;
            _h = _tex->Height;
        }
        else
        {
            SDL_Log("ScrollingTex: Texture not found!");
        }
    }

    void ScrollingTex::Update()
    {
        if (!visible || !_tex)
            return;
        float dt = GameTimes::DeltaTime();
        _offsetX += _scrollX * dt;
        _offsetY += _scrollY * dt;
        if (_w > 0)
            _offsetX = std::fmod(_offsetX, (float)_w);
        if (_h > 0)
            _offsetY = std::fmod(_offsetY, (float)_h);
    }

    void ScrollingTex::Draw(float z)
    {
        if (!visible || !_tex || _w == 0 || _h == 0)
            return;
        // Tile the texture to cover the full screen (160x160).
        int ox = ((int)_offsetX % _w + _w) % _w;
        int oy = ((int)_offsetY % _h + _h) % _h;
        int cols = (Registry::GameConstants::SCREEN_WIDTH_IN_PIXELS / _w) + 2;
        int rows = (Registry::GameConstants::SCREEN_HEIGHT_IN_PIXELS / _h) + 2;
        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                int dx = (int)Position.X + col * _w - ox;
                int dy = (int)Position.Y + row * _h - oy;

                dx += (int)SpriteDrawer::Camera_.Position2D().X;
                dy += (int)SpriteDrawer::Camera_.Position2D().Y;

                Rectangle dest{dx, dy, _w, _h};
                SpriteDrawer::DrawSprite(_tex, dest, nullptr, &Tint, 0.f,
                                         SpriteEffects::None, z);
            }
        }
    }

} // namespace AnodyneSharp::Drawing
