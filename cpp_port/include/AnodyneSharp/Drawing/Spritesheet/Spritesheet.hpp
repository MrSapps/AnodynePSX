#pragma once
#include "AnodyneSharp/Common.hpp"
#include "XNA/Graphics.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"

namespace AnodyneSharp::Drawing::Spritesheet {

class Spritesheet {
public:
    TextureHandle* texHandle = nullptr;
    int Width;
    int Height;

    int NumFrames() const {
        if (!texHandle || !texHandle->Tex()) return 0;
        return (texHandle->Width / Width) * (texHandle->Height / Height);
    }

    Texture2D* GetTex() const { return texHandle ? texHandle->Tex() : nullptr; }

    Spritesheet() : Width(0), Height(0) {}
    Spritesheet(TextureHandle* tex, int width, int height)
        : texHandle(tex), Width(width), Height(height) {}

    Rectangle GetRect(int frame) const {
        if (!texHandle) return {0, 0, Width, Height};
        int w = texHandle->Width > 0 ? texHandle->Width : Width;
        int indexX = frame * Width;
        int indexY = 0;
        if (indexX >= w) {
            indexY = (indexX / w) * Height;
            indexX %= w;
        }
        return {indexX, indexY, Width, Height};
    }
};

} // namespace AnodyneSharp::Drawing::Spritesheet

using AnodyneSharp::Drawing::Spritesheet::Spritesheet;
using AnodyneSharp::Resources::TextureHandle;
