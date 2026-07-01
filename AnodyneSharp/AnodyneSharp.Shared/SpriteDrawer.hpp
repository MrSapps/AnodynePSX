#pragma once

#include "XnaStub.hpp"
#include <string>

namespace AnodyneSharp
{
    class SpriteDrawer
    {
    public:
        static void Initialize(Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice);
        static void Load(Microsoft::Xna::Framework::Graphics::ContentManager& content);
        static void BeginDraw();
        static void EndDraw();
        static void BeginGUIDraw();
        static void EndGUIDraw();
        static void Render();
        static void DrawText(const std::string& text, int x, int y, const Microsoft::Xna::Framework::Color& color, int scale = 2);
        static void DrawFilledRect(int x, int y, int width, int height, const Microsoft::Xna::Framework::Color& color);

    private:
        static Microsoft::Xna::Framework::Graphics::GraphicsDevice* _graphicsDevice;
        static bool _drawing;
        static bool _guidrawing;
    };
}
