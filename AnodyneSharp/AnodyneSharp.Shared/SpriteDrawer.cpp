#include "SpriteDrawer.hpp"
#include <SDL2/SDL.h>
#include <array>
#include <unordered_map>
#include <cctype>
#include <algorithm>

namespace AnodyneSharp
{
    Microsoft::Xna::Framework::Graphics::GraphicsDevice* SpriteDrawer::_graphicsDevice = nullptr;
    bool SpriteDrawer::_drawing = false;
    bool SpriteDrawer::_guidrawing = false;

    void SpriteDrawer::Initialize(Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDevice)
    {
        _graphicsDevice = graphicsDevice;
    }

    void SpriteDrawer::Load(Microsoft::Xna::Framework::Graphics::ContentManager& /*content*/)
    {
        // TODO: load textures, fonts, shaders, and other render assets using the C# ContentManager/MonoGame pipeline.
    }

    void SpriteDrawer::BeginDraw()
    {
        if (_graphicsDevice)
        {
                _graphicsDevice->Clear(Microsoft::Xna::Framework::Color(0, 0, 0, 255));
        }
        _drawing = true;
    }

    void SpriteDrawer::EndDraw()
    {
        _drawing = false;
    }

    void SpriteDrawer::BeginGUIDraw()
    {
        _guidrawing = true;
    }

    void SpriteDrawer::EndGUIDraw()
    {
        _guidrawing = false;
    }

    void SpriteDrawer::Render()
    {
        if (_graphicsDevice)
        {
            _graphicsDevice->Present();
        }
    }

        namespace
        {
            const std::array<uint8_t, 7> EmptyGlyph = { 0, 0, 0, 0, 0, 0, 0 };

            const std::unordered_map<char, std::array<uint8_t, 7>>& GetFontTable()
            {
                static const std::unordered_map<char, std::array<uint8_t, 7>> font = {
                    {'A', {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}},
                    {'B', {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}},
                    {'C', {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}},
                    {'D', {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C}},
                    {'E', {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}},
                    {'F', {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}},
                    {'G', {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F}},
                    {'H', {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}},
                    {'I', {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}},
                    {'J', {0x07,0x02,0x02,0x02,0x12,0x12,0x0C}},
                    {'K', {0x11,0x12,0x14,0x18,0x14,0x12,0x11}},
                    {'L', {0x10,0x10,0x10,0x10,0x10,0x10,0x1F}},
                    {'M', {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}},
                    {'N', {0x11,0x19,0x15,0x13,0x11,0x11,0x11}},
                    {'O', {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}},
                    {'P', {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}},
                    {'Q', {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}},
                    {'R', {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}},
                    {'S', {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}},
                    {'T', {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}},
                    {'U', {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}},
                    {'V', {0x11,0x11,0x11,0x11,0x11,0x0A,0x04}},
                    {'W', {0x11,0x11,0x11,0x15,0x15,0x1B,0x11}},
                    {'X', {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}},
                    {'Y', {0x11,0x11,0x11,0x0A,0x04,0x04,0x04}},
                    {'Z', {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}},
                    {'0', {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}},
                    {'1', {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}},
                    {'2', {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F}},
                    {'3', {0x1F,0x01,0x02,0x06,0x01,0x11,0x0E}},
                    {'4', {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}},
                    {'5', {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}},
                    {'6', {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}},
                    {'7', {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}},
                    {'8', {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}},
                    {'9', {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}},
                    {' ', {0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
                    {'.', {0x00,0x00,0x00,0x00,0x00,0x06,0x06}},
                    {'-', {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}}
                    ,{'>', {0x10,0x08,0x04,0x02,0x04,0x08,0x10}}
                };
                return font;
            }
        }

        void SpriteDrawer::DrawText(const std::string& text, int x, int y, const Microsoft::Xna::Framework::Color& color, int scale)
        {
            if (!_graphicsDevice || !_graphicsDevice->Renderer)
            {
                return;
            }

            SDL_Renderer* renderer = static_cast<SDL_Renderer*>(_graphicsDevice->Renderer);
            SDL_SetRenderDrawColor(renderer, color.R, color.G, color.B, color.A);

            const auto& font = GetFontTable();
            int cursorX = x;
            int cursorY = y;
            const int charWidth = 5;
            const int charHeight = 7;
            const int spacing = 1;

            for (char rawChar : text)
            {
                char c = rawChar;
                if (c == '\n')
                {
                    cursorY += (charHeight + spacing) * scale;
                    cursorX = x;
                    continue;
                }
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                const auto& glyph = font.count(c) ? font.at(c) : EmptyGlyph;

                for (int row = 0; row < charHeight; ++row)
                {
                    uint8_t rowBits = glyph[row];
                    for (int col = 0; col < charWidth; ++col)
                    {
                        if (rowBits & (1 << (charWidth - 1 - col)))
                        {
                            SDL_Rect pixelRect = { cursorX + col * scale, cursorY + row * scale, scale, scale };
                            SDL_RenderFillRect(renderer, &pixelRect);
                        }
                    }
                }

                cursorX += (charWidth + spacing) * scale;
            }
        }

    void SpriteDrawer::DrawFilledRect(int x, int y, int width, int height, const Microsoft::Xna::Framework::Color& color)
    {
        if (!_graphicsDevice || !_graphicsDevice->Renderer)
        {
            return;
        }

        SDL_Renderer* renderer = static_cast<SDL_Renderer*>(_graphicsDevice->Renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, color.R, color.G, color.B, color.A);
        SDL_Rect rect = { x, y, width, height };
        SDL_RenderFillRect(renderer, &rect);
    }
}
