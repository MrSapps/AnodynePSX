#pragma once
#include "AnodyneSharp/Common.hpp"
#include "AnodyneSharp/Drawing/Camera.hpp"
#include "AnodyneSharp/Drawing/Effects/IFullScreenEffect.hpp"
#include "XNA/Graphics.hpp"

namespace AnodyneSharp::Drawing {

class SpriteDrawer {
public:
    static Color        BackColor;
    static SamplerState SamplerState_;
    static Color        FullScreenFade;
    static Texture2D*   SolidTex;
    static SpriteBatch* _spriteBatch;
    static Camera       Camera_;
    static Vector2      _camOffset;  // applied to world-space draws

    static int MaxScale();

    static void Initialize(GraphicsDevice& graphicsDevice);
    static void Load(ContentManager& content);
    static void BeginDraw();
    static void EndDraw();
    static void BeginGUIDraw();
    static void EndGUIDraw();
    static void Render(Effect* effect = nullptr);

    static void DrawSprite(Texture2D* texture, const Rectangle& rect,
                           const Rectangle* sRect = nullptr,
                           const Color* color = nullptr,
                           float rotation = 0.f,
                           SpriteEffects flip = SpriteEffects::None,
                           float Z = 0.f);

    static void DrawSprite(Texture2D* texture, const Rectangle& rect,
                           float Z);

    static void DrawSprite(Texture2D* texture, const Vector2& pos,
                           const Rectangle* sRect = nullptr,
                           const Color* color = nullptr,
                           float rotation = 0.f,
                           float scale = 1.f,
                           float Z = 0.f);

    static Matrix Projection(const Point& screenSize);
    static std::pair<SpriteBatch*, RenderTarget2D*> GetRenderTarget(const Point& size);

private:
    static GraphicsDevice* _graphicsDevice;
};

} // namespace AnodyneSharp::Drawing

using AnodyneSharp::Drawing::SpriteDrawer;
