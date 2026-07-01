// Software implementation of BlendEffect (overlay blend)
#include "AnodyneSharp/Drawing/Effects/Effects.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "SDL3Context.hpp"
#include <algorithm>
#include <cstring>

namespace AnodyneSharp::Drawing::Effects {

// Scale the overlay texture's raw RGBA cpuPixels to 160x180 and convert to ARGB8888
// (ARGB8888 memory layout on little-endian: byte[0]=B, byte[1]=G, byte[2]=R, byte[3]=A)
void BlendEffect::SetTex(const std::string& texName) {
    if (texName.empty()) {
        tex = nullptr;
        hard_light = false;
        _bakedPixels.clear();
        return;
    }

    tex = Resources::ResourceManager::GetTexture(texName, true);
    hard_light = (texName.find("hardlight") != std::string::npos);

    if (!tex || tex->cpuPixels.empty()) {
        _bakedPixels.clear();
        return;
    }

    constexpr int GW = SDL3Context::GAME_W;
    constexpr int GH = SDL3Context::GAME_H;

    _bakedPixels.resize(GW * GH * 4);

    const float xScale = (float)tex->cpuWidth  / (float)GW;
    const float yScale = (float)tex->cpuHeight / (float)GH;

    for (int y = 0; y < GH; y++) {
        for (int x = 0; x < GW; x++) {
            int sx = std::min((int)(x * xScale), tex->cpuWidth  - 1);
            int sy = std::min((int)(y * yScale), tex->cpuHeight - 1);

            // Source: RGBA bytes (stb_image: R=0, G=1, B=2, A=3)
            const uint8_t* src = tex->cpuPixels.data() + (sy * tex->cpuWidth + sx) * 4;
            // Dest: ARGB8888 on LE (B=0, G=1, R=2, A=3)
            uint8_t* dst = _bakedPixels.data() + (y * GW + x) * 4;
            dst[0] = src[2]; // B
            dst[1] = src[1]; // G
            dst[2] = src[0]; // R
            dst[3] = src[3]; // A
        }
    }
}

} // namespace AnodyneSharp::Drawing::Effects
