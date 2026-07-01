#pragma once
#include "AnodyneSharp/Common.hpp"
#include "XNA/Graphics.hpp"

namespace AnodyneSharp::Drawing::Effects {

class IFullScreenEffect {
public:
    virtual ~IFullScreenEffect() = default;
    virtual bool Active() const = 0;
    virtual void Deactivate() = 0;
    virtual void Load(ContentManager& content, GraphicsDevice& graphicsDevice) = 0;
    virtual void Render(SpriteBatch& batch, Texture2D& screen) = 0;
    virtual void Update() {}
};

} // namespace AnodyneSharp::Drawing::Effects

using AnodyneSharp::Drawing::Effects::IFullScreenEffect;
