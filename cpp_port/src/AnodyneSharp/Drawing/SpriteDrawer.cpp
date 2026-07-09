#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Drawing/Camera.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "SDL3Context.hpp"
#include <SDL3/SDL.h>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <vector>

// ============================================================================
// CPU pixel-effect helpers  (ARGB8888 little-endian: bytes [B, G, R, A])
// ============================================================================
namespace {
    constexpr int GW   = SDL3Context::GAME_W;   // 160
    constexpr int GH   = SDL3Context::GAME_H;   // 180
    constexpr int GBUF = GW * GH * 4;            // 115 200 bytes

    // Per-frame scratch buffers (avoid per-frame heap allocation)
    std::vector<uint8_t> s_pixBuf;
    std::vector<uint8_t> s_tmpBuf;

    // BT.601 luma: 0.299*R + 0.587*G + 0.114*B  (ARGB p[2]=R, p[1]=G, p[0]=B)
    inline void applyGrayscale(uint8_t* p, int count) {
        for (int i = 0; i < count; i++, p += 4) {
            uint8_t gray = (uint8_t)(0.114f * p[0] + 0.587f * p[1] + 0.299f * p[2]);
            p[0] = p[1] = p[2] = gray;
        }
    }

    // Deterministic XOR-shift noise keyed by step (0-3)
    inline void applyStatic(uint8_t* p, int count, int step) {
        uint32_t seed = (uint32_t)(step * 0x9E3779B9u + 12345u);
        for (int i = 0; i < count; i++, p += 4) {
            seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
            if ((seed & 3u) == 0u) {   // ~25 % pixels get noise
                uint8_t noise = (uint8_t)((seed >> 8) & 0xFFu);
                p[0] = p[1] = p[2] = noise;
            }
        }
    }

    // Horizontal pixel-block averaging (stride = block size in pixels)
    inline void applyPixelation(uint8_t* p, int stride) {
        if (stride <= 1) return;
        s_tmpBuf.assign(p, p + GBUF);
        const uint8_t* src = s_tmpBuf.data();
        for (int by = 0; by < GH; by += stride) {
            for (int bx = 0; bx < GW; bx += stride) {
                const uint8_t* s = src + (by * GW + bx) * 4;
                uint8_t b = s[0], g = s[1], r = s[2], a = s[3];
                for (int dy = 0; dy < stride && by + dy < GH; dy++)
                    for (int dx = 0; dx < stride && bx + dx < GW; dx++) {
                        uint8_t* d = p + ((by + dy) * GW + (bx + dx)) * 4;
                        d[0] = b; d[1] = g; d[2] = r; d[3] = a;
                    }
            }
        }
    }

    // Sinusoidal horizontal row-shift
    inline void applyWave(uint8_t* p, int phase) {
        constexpr float AMP  = 3.f;
        constexpr float FREQ = 2.f * 3.14159265f / 180.f;
        s_tmpBuf.assign(p, p + GBUF);
        const uint8_t* src = s_tmpBuf.data();
        for (int y = 0; y < GH; y++) {
            int shift = (int)(AMP * std::sin((float)(y + phase) * FREQ));
            for (int x = 0; x < GW; x++) {
                int sx = ((x - shift) % GW + GW) % GW;
                uint8_t*       d = p   + (y * GW + x)  * 4;
                const uint8_t* s = src + (y * GW + sx) * 4;
                d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
            }
        }
    }
} // anonymous namespace

namespace AnodyneSharp::Drawing {

Color        SpriteDrawer::BackColor     = Color::Black;
SamplerState SpriteDrawer::SamplerState_ = SamplerState::PointClamp;
Color        SpriteDrawer::FullScreenFade = Color::White;
Texture2D*   SpriteDrawer::SolidTex      = nullptr;
SpriteBatch* SpriteDrawer::_spriteBatch  = nullptr;
Camera       SpriteDrawer::Camera_;
Vector2      SpriteDrawer::_camOffset    = {0.f, 0.f};
GraphicsDevice* SpriteDrawer::_graphicsDevice = nullptr;

int SpriteDrawer::MaxScale() { return 4; }

void SpriteDrawer::Initialize(GraphicsDevice& graphicsDevice) {
    _graphicsDevice = &graphicsDevice;
    // Wire the global renderer into GraphicsDevice
    _graphicsDevice->sdlRenderer = SDL3Context::Renderer;
    _spriteBatch = new SpriteBatch(_graphicsDevice);
}

void SpriteDrawer::Load(ContentManager& /*content*/) {
    // Create SpriteBatch if Initialize() was never called (renderer now available)
    if (!_spriteBatch) {
        _spriteBatch = new SpriteBatch(nullptr); // uses SDL3Context::Renderer fallback
        SDL_Log("SpriteDrawer::Load: created SpriteBatch renderer=%p", (void*)SDL3Context::Renderer);
    }

    // Create a 1×1 white texture for solid-color drawing
    if (SDL3Context::Renderer) {
        SDL_Texture* t = SDL_CreateTexture(
            SDL3Context::Renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STATIC, 1, 1);
        if (t) {
            Uint32 white = 0xFFFFFFFF;
            SDL_UpdateTexture(t, nullptr, &white, 4);
            SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
            SolidTex = new Texture2D();
            SolidTex->AttachSDL(t, 1, 1);

            SDL_SetTextureScaleMode(t, SDL_SCALEMODE_NEAREST);
        }
    }
}

void SpriteDrawer::BeginDraw() {
    Camera_.Recalc();
    _camOffset = Camera_.Position2D();
    // Redirect all scene draws to the offscreen render target and clear it
    if (SDL3Context::OffscreenTarget) {
        SDL_SetRenderTarget(SDL3Context::Renderer, SDL3Context::OffscreenTarget);
        SDL_SetRenderDrawColor(SDL3Context::Renderer, 0, 0, 0, 255);
        SDL_RenderClear(SDL3Context::Renderer);
    }
    if (_spriteBatch) _spriteBatch->Begin();
}
void SpriteDrawer::EndDraw() {
    
    if (_spriteBatch) 
    {
        _spriteBatch->End();
    }

    

}

void SpriteDrawer::BeginGUIDraw() {
    _camOffset = {0.f, 0.f};
    if (_spriteBatch) _spriteBatch->Begin();
}

void SpriteDrawer::EndGUIDraw()   
{
     if (_spriteBatch) _spriteBatch->End(); 
}

void SpriteDrawer::Render(Effect* /*effect*/) {
    auto* renderer  = SDL3Context::Renderer;
    if (!renderer) return;
    auto* offscreen = SDL3Context::OffscreenTarget;
    auto* effectTex = SDL3Context::EffectTexture;

    /*
    static int _rc = 0; ++_rc;
    if (_rc <= 5 || _rc % 60 == 0)
        SDL_Log("Render #%d flash=%.2f offscreen=%p",
            _rc, AnodyneSharp::Registry::GlobalState::flash.GetAlpha(), (void*)offscreen);
    */

    using GS = AnodyneSharp::Registry::GlobalState;

    // --- 1. FullScreenFade overlay → still targeting offscreen ---
    if (FullScreenFade.R < 255 || FullScreenFade.G < 255 || FullScreenFade.B < 255) {
        if (SolidTex && _spriteBatch) {
            SDL_SetRenderTarget(renderer, offscreen);
            _spriteBatch->Begin();
            uint8_t alpha = (uint8_t)(255 - std::min({FullScreenFade.R, FullScreenFade.G, FullScreenFade.B}));
            Color overlay(0, 0, 0, (int)alpha);
            Rectangle full{0, 0, GW, GH};
            _spriteBatch->Draw(SolidTex, full, nullptr, overlay, 0.f, Vector2{}, SpriteEffects::None, 0.f);
            _spriteBatch->End();
        }
    }

    // --- 2. Determine which pixel effects are active ---
    bool needPixels = GS::pixelation.Active() || GS::staticEffect.Active() || GS::wave.Active()
                   || GS::fgBlend.Active() || GS::extraBlend.Active();
    bool needGlitch = GS::glitch.Active();

    // --- 3. CPU pixel processing (read offscreen → transform → upload) ---
    if (needPixels && offscreen && effectTex) {
        SDL_Surface* surf = SDL_RenderReadPixels(renderer, nullptr);
        if (surf) {
            SDL_Surface* conv = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ARGB8888);
            SDL_DestroySurface(surf);
            if (conv) {
                s_pixBuf.resize(GBUF);
                memcpy(s_pixBuf.data(), conv->pixels, GBUF);
                SDL_DestroySurface(conv);

                uint8_t* px = s_pixBuf.data();
                if (GS::wave.Active())          applyWave(px, GS::wave.GetPhase());
                if (GS::pixelation.Active())    applyPixelation(px, GS::pixelation.GetPixelSize());
                if (GS::staticEffect.Active()) {
                    applyGrayscale(px, GW * GH);
                    applyStatic(px, GW * GH, GS::staticEffect.GetStep());
                }

                // Overlay blend effects (FG_Blend / ExtraBlend)
                auto applyBlend = [&](const Drawing::Effects::BlendEffect& effect) {
                    if (!effect.Active()) return;
                    const auto& overlay = effect.GetBakedPixels();
                    if ((int)overlay.size() < GW * GH * 4) return;
                    const bool hl = effect.IsHardLight();
                    for (int i = 0; i < GW * GH; i++) {
                        uint8_t* s = px + i * 4;             // screen: B,G,R,A
                        const uint8_t* o = overlay.data() + i * 4; // overlay: B,G,R,A
                        float oa = o[3] / 255.f;
                        if (oa == 0.f) continue;
                        for (int c = 0; c < 3; c++) {        // channels: B, G, R
                            float sc = s[c] / 255.f;
                            float oc = o[c] / 255.f;
                            float blended;
                            if (!hl) {
                                // Overlay mode: base=screen, blend=overlay
                                blended = (sc < 0.5f)
                                    ? (2.f * sc * oc)
                                    : (1.f - 2.f * (1.f - sc) * (1.f - oc));
                            } else {
                                // Hard-light: base=overlay, blend=screen
                                blended = (oc < 0.5f)
                                    ? (2.f * sc * oc)
                                    : (1.f - 2.f * (1.f - oc) * (1.f - sc));
                            }
                            s[c] = (uint8_t)std::min(255.f, (blended * oa + sc * (1.f - oa)) * 255.f + 0.5f);
                        }
                    }
                };
                applyBlend(GS::fgBlend);
                applyBlend(GS::extraBlend);

                SDL_UpdateTexture(effectTex, nullptr, s_pixBuf.data(), GW * 4);
            }
        }
    }

    // --- 4. Switch render target to screen ---
    // DEBUG: probe center pixel of offscreen to see if content was drawn
    /*
    {
        static int _probe = 0;
        if (++_probe <= 5 || _probe % 120 == 0) {
            SDL_Surface* ps = SDL_RenderReadPixels(renderer, nullptr);
            if (ps) {
                SDL_Surface* pc = SDL_ConvertSurface(ps, SDL_PIXELFORMAT_ARGB8888);
                SDL_DestroySurface(ps);
                if (pc) {
                    uint32_t* px = (uint32_t*)pc->pixels;
                    SDL_Log("Probe #%d: offscreen center=%08X topleft=%08X", _probe,
                        px[90*GW+80], px[0]);
                    SDL_DestroySurface(pc);
                }
            }
        }
    }*/

    SDL_SetRenderTarget(renderer, nullptr);

    // --- 5. Blit processed or raw offscreen to screen ---
    if (needPixels && effectTex) {
        SDL_SetTextureBlendMode(effectTex, SDL_BLENDMODE_NONE);
        SDL_RenderTexture(renderer, effectTex, nullptr, nullptr);
    } else if (offscreen) {
        SDL_SetTextureBlendMode(offscreen, SDL_BLENDMODE_NONE);
        SDL_RenderTexture(renderer, offscreen, nullptr, nullptr);
    }

    // --- 6. Glitch: copy random rects from offscreen on top of screen ---
    if (needGlitch && offscreen) {
        SDL_SetTextureBlendMode(offscreen, SDL_BLENDMODE_NONE);
        for (const auto& gr : GS::glitch.GetRects()) {
            SDL_FRect srcR = { (float)gr.srcX, (float)gr.srcY, (float)gr.srcW, (float)gr.srcH };
            SDL_FRect dstR = { (float)gr.dstX, (float)gr.dstY, (float)gr.srcW, (float)gr.srcH };
            SDL_RenderTexture(renderer, offscreen, &srcR, &dstR);
        }
    }

    // --- 7. Flash overlay: colored rect in logical screen space ---
    if (GS::flash.Active()) {
        Color fc = GS::flash.GetFlashColor();
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, fc.R, fc.G, fc.B, fc.A);
        SDL_FRect full = { 0.f, 0.f, (float)GW, (float)GH };
        SDL_RenderFillRect(renderer, &full);
    }
}

void SpriteDrawer::DrawSprite(Texture2D* texture, const Rectangle& rect,
                               const Rectangle* sRect,
                               const Color* color,
                               float rotation,
                               SpriteEffects flip,
                               float Z) {
    if (!_spriteBatch || !texture) return;
    Color tint = color ? *color : Color::White;
    Rectangle dst{rect.X - (int)_camOffset.X, rect.Y - (int)_camOffset.Y, rect.Width, rect.Height};
    _spriteBatch->Draw(texture, dst, sRect, tint, rotation, Vector2{}, flip, Z);
}

void SpriteDrawer::DrawSprite(Texture2D* texture, const Rectangle& rect,
                           float Z)
{
    DrawSprite(texture, rect, nullptr, nullptr, 0.f, SpriteEffects::None, Z);
}

void SpriteDrawer::DrawSprite(Texture2D* texture, const Vector2& pos,
                               const Rectangle* sRect,
                               const Color* color,
                               float rotation,
                               float scale,
                               float Z) {
    if (!_spriteBatch || !texture) return;
    Color tint = color ? *color : Color::White;
    Vector2 adjusted{pos.X - _camOffset.X, pos.Y - _camOffset.Y};
    _spriteBatch->Draw(texture, adjusted, sRect, tint, rotation, Vector2{}, scale, SpriteEffects::None, Z);
}

Matrix SpriteDrawer::Projection(const Point& /*screenSize*/) {
    return Matrix::Identity();
}

std::pair<SpriteBatch*, RenderTarget2D*> SpriteDrawer::GetRenderTarget(const Point& size) {
    return {_spriteBatch, nullptr};
}

} // namespace AnodyneSharp::Drawing
