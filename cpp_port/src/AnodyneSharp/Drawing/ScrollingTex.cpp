#include "AnodyneSharp/Drawing/ScrollingTex.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/GameTimes.hpp"

namespace AnodyneSharp::Drawing
{

    void ScrollingTex::Load(const std::string &texName, Vector2 velocity, DrawOrder drawOrder)
    {
        _tex = Resources::ResourceManager::GetTexture(texName);
        if (!_tex)
        {
            SDL_Log("ScrollingTex: Texture not found!");
        }

        _texName = texName;
        Position = velocity;
        _layer = drawOrder;
    }

    void ScrollingTex::Draw()
    {
        Vector2 topleft = Position + SpriteDrawer::Camera_.Position2D();
        Rectangle pos((int)topleft.X, (int)topleft.Y, _tex->Width, _tex->Height);
        SpriteDrawer::DrawSprite(_tex, pos, DrawingUtilities::GetDrawingZ(_layer, 0));
    }

    void ScrollingTex::Update()
    {
        Position += _velocity * GameTimes::DeltaTime();
        if (Position.X < -_tex->Width / 2)
        {
            Position.X = 0;
        }

        if (Position.Y > 0)
        {
            Position.Y = -_tex->Height / 2;
        }
        else if (Position.Y < -_tex->Height / 2)
        {
            Position.Y = 0;
        }
    }

} // namespace AnodyneSharp::Drawing
