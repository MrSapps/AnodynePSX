// SDL3 backend — implements all XNA stub types with real SDL3 calls
#define SDL_MAIN_USE_CALLBACKS 0
#include "SDL3Context.hpp"
#include "XNA/Framework.hpp"
#include "XNA/Graphics.hpp"
#include "XNA/Input.hpp"
#include "XNA/Audio.hpp"
#include <algorithm>
#include <stdexcept>
#include <cmath>

// ============================================================================
// SDL3Context globals
// ============================================================================
namespace SDL3Context {
    SDL_Window*       Window          = nullptr;
    SDL_Renderer*     Renderer        = nullptr;
    SDL_AudioDeviceID AudioDevice     = 0;
    SDL_AudioSpec     AudioSpec       = {};
    SDL_Texture*      OffscreenTarget = nullptr;
    SDL_Texture*      EffectTexture   = nullptr;
}

// ============================================================================
// Game::Run() — creates SDL3 window + runs the game loop
// ============================================================================
void Game::Run() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    // Create window (4× scaled game viewport)
    SDL3Context::Window = SDL_CreateWindow(
        "Anodyne Sharp",
        SDL3Context::WIN_W, SDL3Context::WIN_H,
        0);
    if (!SDL3Context::Window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit(); return;
    }

    SDL_SetWindowTitle(SDL3Context::Window, "Anodyne Blunt"); // V Funny joke

    // Create hardware-accelerated renderer
    SDL3Context::Renderer = SDL_CreateRenderer(SDL3Context::Window, nullptr);
    if (!SDL3Context::Renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(SDL3Context::Window);
        SDL_Quit(); return;
    }

    // Vsync keeps RenderPresent locked to display refresh so DeltaTime stays accurate
    SDL_SetRenderVSync(SDL3Context::Renderer, 1);

    // Logical size: game coordinates are in game pixels; SDL scales to window
    SDL_SetRenderLogicalPresentation(
        SDL3Context::Renderer,
        SDL3Context::GAME_W, SDL3Context::GAME_H,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    // Create offscreen render target (all scene draws redirected here)
    SDL3Context::OffscreenTarget = SDL_CreateTexture(
        SDL3Context::Renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        SDL3Context::GAME_W, SDL3Context::GAME_H);
    if (SDL3Context::OffscreenTarget)
    {
        SDL_SetTextureScaleMode(SDL3Context::OffscreenTarget, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(SDL3Context::OffscreenTarget, SDL_BLENDMODE_BLEND);
    }

    // Create streaming texture for CPU pixel-effect upload
    SDL3Context::EffectTexture = SDL_CreateTexture(
        SDL3Context::Renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SDL3Context::GAME_W, SDL3Context::GAME_H);
    if (SDL3Context::EffectTexture)
    {
        SDL_SetTextureScaleMode(SDL3Context::EffectTexture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(SDL3Context::EffectTexture, SDL_BLENDMODE_NONE);
    }

    // Audio device
    SDL3Context::AudioSpec = { SDL_AUDIO_F32LE, 2, 44100 };
    SDL3Context::AudioDevice = SDL_OpenAudioDevice(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &SDL3Context::AudioSpec);

    Initialize();
    LoadContent();

    constexpr double TARGET_DT = 1.0 / 60.0;
    Uint64 lastTick = SDL_GetPerformanceCounter();
    Uint64 freq     = SDL_GetPerformanceFrequency();
    double accumulator = 0.0;

    _running = true;
    while (_running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT)           _running = false;
            if (ev.type == SDL_EVENT_WINDOW_FOCUS_GAINED) IsActive = true;
            if (ev.type == SDL_EVENT_WINDOW_FOCUS_LOST)   IsActive = false;
        }

        Uint64 now = SDL_GetPerformanceCounter();
        double dt  = (double)(now - lastTick) / (double)freq;
        lastTick   = now;
        if (dt > 0.1) dt = 0.1;   // cap spiral-of-death

        accumulator += dt;
        while (accumulator >= TARGET_DT) {
            GameTime gt;
            gt.ElapsedGameTime = TimeSpan::FromSeconds(TARGET_DT);
            Update(gt);
            accumulator -= TARGET_DT;
        }

        // Draw
        SDL_SetRenderDrawColor(SDL3Context::Renderer, 0, 0, 0, 255);
        SDL_RenderClear(SDL3Context::Renderer);

        GameTime drawGt;
        drawGt.ElapsedGameTime = TimeSpan::FromSeconds(TARGET_DT);
        Draw(drawGt);

        SDL_RenderPresent(SDL3Context::Renderer);
    }

    if (SDL3Context::AudioDevice) SDL_CloseAudioDevice(SDL3Context::AudioDevice);
    SDL_DestroyRenderer(SDL3Context::Renderer);
    SDL_DestroyWindow(SDL3Context::Window);
    SDL_Quit();
}

// ============================================================================
// Texture2D
// ============================================================================
using Microsoft::Xna::Framework::Graphics::Texture2D;
using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
using Microsoft::Xna::Framework::Graphics::SpriteBatch;
using Microsoft::Xna::Framework::Graphics::DrawCall;

Texture2D::Texture2D(GraphicsDevice* /*dev*/, int w, int h)
    : Width(w), Height(h), Bounds(0, 0, w, h) {}

void Texture2D::AttachSDL(SDL_Texture* t, int w, int h) {
    if (sdlTex && sdlTex != t) SDL_DestroyTexture(sdlTex);
    sdlTex = t;
    Width = w; Height = h;
    Bounds = {0, 0, w, h};
}

Texture2D::~Texture2D() {
    if (sdlTex) { SDL_DestroyTexture(sdlTex); sdlTex = nullptr; }
}

// ============================================================================
// RenderTarget2D
// ============================================================================
RenderTarget2D::RenderTarget2D(GraphicsDevice* dev, int w, int h)
    : Texture2D(dev, w, h)
{
    if (dev && dev->sdlRenderer) {
        sdlTex = SDL_CreateTexture(dev->sdlRenderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET, w, h);
        if (sdlTex) 
        {
            SDL_SetTextureScaleMode(sdlTex, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(sdlTex, SDL_BLENDMODE_BLEND);
        }
    }
}

// ============================================================================
// GraphicsDevice
// ============================================================================
void GraphicsDevice::Clear(const Color& color) {
    if (!sdlRenderer) return;
    SDL_SetRenderDrawColor(sdlRenderer,
        (Uint8)color.R, (Uint8)color.G, (Uint8)color.B, (Uint8)color.A);
    SDL_RenderClear(sdlRenderer);
}

void GraphicsDevice::SetRenderTarget(RenderTarget2D* target) {
    if (!sdlRenderer) return;
    currentRT = target ? target->sdlTex : nullptr;
    SDL_SetRenderTarget(sdlRenderer, currentRT);
}

// ============================================================================
// SpriteBatch
// ============================================================================
SpriteBatch::SpriteBatch(GraphicsDevice* dev) {
    if (dev) {
        _gdev = dev;
        _renderer = dev->sdlRenderer;
    }
    if (!_renderer) _renderer = SDL3Context::Renderer;
}

static SDL_FlipMode ToFlip(SpriteEffects e) {
    int f = (int)e;
    if (f == 1) return SDL_FLIP_HORIZONTAL;
    if (f == 2) return SDL_FLIP_VERTICAL;
    if (f == 3) return (SDL_FlipMode)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
    return SDL_FLIP_NONE;
}

void SpriteBatch::Draw(Texture2D* texture, const Rectangle& dest, const Color& color) {
    Draw(texture, dest, nullptr, color, 0.f, Vector2{}, SpriteEffects::None, 0.f);
}

void SpriteBatch::Draw(Texture2D* texture, const Rectangle& dest, const Rectangle* sourceRect,
                       const Color& color, float rotation, const Vector2& origin,
                       SpriteEffects effects, float layerDepth)
{
    if (!texture || !texture->sdlTex) return;
    DrawCall dc;
    dc.tex = texture->sdlTex;
    if (sourceRect)
        dc.src = { (float)sourceRect->X, (float)sourceRect->Y,
                   (float)sourceRect->Width, (float)sourceRect->Height };
    else
        dc.src = { 0.f, 0.f, (float)texture->Width, (float)texture->Height };
    dc.dst    = { (float)dest.X, (float)dest.Y, (float)dest.Width, (float)dest.Height };
    dc.origin = { origin.X, origin.Y };
    dc.angleDeg = (double)(rotation * (180.0f / 3.14159265358979f));
    dc.flip   = ToFlip(effects);
    dc.color  = { (Uint8)color.R, (Uint8)color.G, (Uint8)color.B, (Uint8)color.A };
    dc.z      = layerDepth;
    _calls.push_back(dc);
}

void SpriteBatch::Draw(Texture2D* texture, const Vector2& pos, const Color& color) {
    Rectangle dest{ (int)pos.X, (int)pos.Y, texture ? texture->Width : 0, texture ? texture->Height : 0 };
    Draw(texture, dest, color);
}

void SpriteBatch::Draw(Texture2D* texture, const Vector2& pos, const Rectangle* sourceRect,
                       const Color& color, float rotation, const Vector2& origin,
                       float scale, SpriteEffects effects, float layerDepth)
{
    int w = sourceRect ? sourceRect->Width  : (texture ? texture->Width  : 0);
    int h = sourceRect ? sourceRect->Height : (texture ? texture->Height : 0);
    Rectangle dest{ (int)pos.X, (int)pos.Y, (int)(w * scale), (int)(h * scale) };
    Draw(texture, dest, sourceRect, color, rotation, origin, effects, layerDepth);
}

// Clip a src+dst pair to the render-target area [0,maxW]x[0,maxH].
// UV ratios are preserved so the visible portion of the source texture
// maps correctly.  Returns false if the draw is entirely off-screen.
static bool ClipSrcDst(SDL_FRect& src, SDL_FRect& dst,
                        float maxW = 160.f, float maxH = 180.f)
{
    if (dst.w <= 0.f || dst.h <= 0.f) return false;

    // clip top
    if (dst.y < 0.f) {
        float clip = -dst.y;
        if (clip >= dst.h) return false;
        float uv = src.h / dst.h;
        src.y += clip * uv;  src.h -= clip * uv;
        dst.h -= clip;       dst.y  = 0.f;
    }
    // clip left
    if (dst.x < 0.f) {
        float clip = -dst.x;
        if (clip >= dst.w) return false;
        float uv = src.w / dst.w;
        src.x += clip * uv;  src.w -= clip * uv;
        dst.w -= clip;       dst.x  = 0.f;
    }
    // clip right
    if (dst.x + dst.w > maxW) {
        float clip = (dst.x + dst.w) - maxW;
        if (clip >= dst.w) return false;
        src.w -= clip * (src.w / dst.w);
        dst.w -= clip;
    }
    // clip bottom
    if (dst.y + dst.h > maxH) {
        float clip = (dst.y + dst.h) - maxH;
        if (clip >= dst.h) return false;
        src.h -= clip * (src.h / dst.h);
        dst.h -= clip;
    }
    return dst.w > 0.f && dst.h > 0.f;
}

void SpriteBatch::End() {
    if (!_renderer) { _calls.clear(); return; }
    
    /*
    static int _batchN = 0; ++_batchN;
    if (_batchN <= 20 || _batchN % 120 == 0)
        SDL_Log("Batch::End #%d  calls=%zu", _batchN, _calls.size());
    */

    // Sort: higher Z = further back (XNA BackToFront: Z=1 is back, Z=0 is front)
    // Draw back to front = sort descending by Z
    std::stable_sort(_calls.begin(), _calls.end(),
        [](const DrawCall& a, const DrawCall& b){ return a.z > b.z; });

    for (auto& dc : _calls) {
        SDL_SetTextureColorMod(dc.tex, dc.color.r, dc.color.g, dc.color.b);
        SDL_SetTextureAlphaMod(dc.tex, dc.color.a);
        SDL_SetTextureBlendMode(dc.tex, SDL_BLENDMODE_BLEND);

        if (dc.angleDeg == 0.0 && dc.flip == SDL_FLIP_NONE) {
            // Non-rotated: use SDL_RenderTexture with explicit src/dst clipping.
            // SDL_RenderTextureRotated can skip draws when the rotation center
            // (dc.origin at the dst top-left) falls outside the render target —
            // clipping manually ensures partial off-screen tiles scroll smoothly.
            SDL_FRect src = dc.src, dst = dc.dst;
            if (ClipSrcDst(src, dst)) {
                SDL_RenderTexture(_renderer, dc.tex, &src, &dst);
            }
        } else {
            SDL_RenderTextureRotated(_renderer, dc.tex,
                &dc.src, &dc.dst, dc.angleDeg, &dc.origin, dc.flip);
        }
    }
    _calls.clear();
}

// ============================================================================
// Input: XNA Keys → SDL_Scancode mapping
// ============================================================================
using Microsoft::Xna::Framework::Input::Keys;
using Microsoft::Xna::Framework::Input::KeyboardState;
using Microsoft::Xna::Framework::Input::GamePadState;
using Microsoft::Xna::Framework::Input::GamePad;
using Microsoft::Xna::Framework::Input::Buttons;
using Microsoft::Xna::Framework::Input::PlayerIndex;

namespace Microsoft::Xna::Framework::Input {
SDL_Scancode XNAKeyToScancode(Keys k) {
    // Windows VK codes → SDL_Scancode
    switch (k) {
    // Letters A-Z  (VK 65-90 == ASCII)
    case Keys::A: return SDL_SCANCODE_A;  case Keys::B: return SDL_SCANCODE_B;
    case Keys::C: return SDL_SCANCODE_C;  case Keys::D: return SDL_SCANCODE_D;
    case Keys::E: return SDL_SCANCODE_E;  case Keys::F: return SDL_SCANCODE_F;
    case Keys::G: return SDL_SCANCODE_G;  case Keys::H: return SDL_SCANCODE_H;
    case Keys::I: return SDL_SCANCODE_I;  case Keys::J: return SDL_SCANCODE_J;
    case Keys::K: return SDL_SCANCODE_K;  case Keys::L: return SDL_SCANCODE_L;
    case Keys::M: return SDL_SCANCODE_M;  case Keys::N: return SDL_SCANCODE_N;
    case Keys::O: return SDL_SCANCODE_O;  case Keys::P: return SDL_SCANCODE_P;
    case Keys::Q: return SDL_SCANCODE_Q;  case Keys::R: return SDL_SCANCODE_R;
    case Keys::S: return SDL_SCANCODE_S;  case Keys::T: return SDL_SCANCODE_T;
    case Keys::U: return SDL_SCANCODE_U;  case Keys::V: return SDL_SCANCODE_V;
    case Keys::W: return SDL_SCANCODE_W;  case Keys::X: return SDL_SCANCODE_X;
    case Keys::Y: return SDL_SCANCODE_Y;  case Keys::Z: return SDL_SCANCODE_Z;
    // Digits
    case Keys::D0: return SDL_SCANCODE_0; case Keys::D1: return SDL_SCANCODE_1;
    case Keys::D2: return SDL_SCANCODE_2; case Keys::D3: return SDL_SCANCODE_3;
    case Keys::D4: return SDL_SCANCODE_4; case Keys::D5: return SDL_SCANCODE_5;
    case Keys::D6: return SDL_SCANCODE_6; case Keys::D7: return SDL_SCANCODE_7;
    case Keys::D8: return SDL_SCANCODE_8; case Keys::D9: return SDL_SCANCODE_9;
    // Arrow keys
    case Keys::Left:  return SDL_SCANCODE_LEFT;
    case Keys::Right: return SDL_SCANCODE_RIGHT;
    case Keys::Up:    return SDL_SCANCODE_UP;
    case Keys::Down:  return SDL_SCANCODE_DOWN;
    // Common
    case Keys::Enter:   return SDL_SCANCODE_RETURN;
    case Keys::Escape:  return SDL_SCANCODE_ESCAPE;
    case Keys::Space:   return SDL_SCANCODE_SPACE;
    case Keys::Back:    return SDL_SCANCODE_BACKSPACE;
    case Keys::Tab:     return SDL_SCANCODE_TAB;
    case Keys::Delete:  return SDL_SCANCODE_DELETE;
    case Keys::Insert:  return SDL_SCANCODE_INSERT;
    case Keys::Home:    return SDL_SCANCODE_HOME;
    case Keys::End:     return SDL_SCANCODE_END;
    case Keys::PageUp:  return SDL_SCANCODE_PAGEUP;
    case Keys::PageDown:return SDL_SCANCODE_PAGEDOWN;
    // Function keys
    case Keys::F1:  return SDL_SCANCODE_F1;  case Keys::F2:  return SDL_SCANCODE_F2;
    case Keys::F3:  return SDL_SCANCODE_F3;  case Keys::F4:  return SDL_SCANCODE_F4;
    case Keys::F5:  return SDL_SCANCODE_F5;  case Keys::F6:  return SDL_SCANCODE_F6;
    case Keys::F7:  return SDL_SCANCODE_F7;  case Keys::F8:  return SDL_SCANCODE_F8;
    case Keys::F9:  return SDL_SCANCODE_F9;  case Keys::F10: return SDL_SCANCODE_F10;
    case Keys::F11: return SDL_SCANCODE_F11; case Keys::F12: return SDL_SCANCODE_F12;
    // Modifiers
    case Keys::LeftShift:   return SDL_SCANCODE_LSHIFT;
    case Keys::RightShift:  return SDL_SCANCODE_RSHIFT;
    case Keys::LeftControl: return SDL_SCANCODE_LCTRL;
    case Keys::RightControl:return SDL_SCANCODE_RCTRL;
    case Keys::LeftAlt:     return SDL_SCANCODE_LALT;
    case Keys::RightAlt:    return SDL_SCANCODE_RALT;
    // Numpad
    case Keys::NumPad0: return SDL_SCANCODE_KP_0; case Keys::NumPad1: return SDL_SCANCODE_KP_1;
    case Keys::NumPad2: return SDL_SCANCODE_KP_2; case Keys::NumPad3: return SDL_SCANCODE_KP_3;
    case Keys::NumPad4: return SDL_SCANCODE_KP_4; case Keys::NumPad5: return SDL_SCANCODE_KP_5;
    case Keys::NumPad6: return SDL_SCANCODE_KP_6; case Keys::NumPad7: return SDL_SCANCODE_KP_7;
    case Keys::NumPad8: return SDL_SCANCODE_KP_8; case Keys::NumPad9: return SDL_SCANCODE_KP_9;
    case Keys::Multiply: return SDL_SCANCODE_KP_MULTIPLY;
    case Keys::Add:      return SDL_SCANCODE_KP_PLUS;
    case Keys::Subtract: return SDL_SCANCODE_KP_MINUS;
    case Keys::Decimal:  return SDL_SCANCODE_KP_PERIOD;
    case Keys::Divide:   return SDL_SCANCODE_KP_DIVIDE;
    // Punctuation
    case Keys::OemPeriod:       return SDL_SCANCODE_PERIOD;
    case Keys::OemComma:        return SDL_SCANCODE_COMMA;
    case Keys::OemMinus:        return SDL_SCANCODE_MINUS;
    case Keys::OemPlus:         return SDL_SCANCODE_EQUALS;
    case Keys::OemSemicolon:    return SDL_SCANCODE_SEMICOLON;
    case Keys::OemTilde:        return SDL_SCANCODE_GRAVE;
    case Keys::OemOpenBrackets: return SDL_SCANCODE_LEFTBRACKET;
    case Keys::OemCloseBrackets:return SDL_SCANCODE_RIGHTBRACKET;
    case Keys::OemPipe:         return SDL_SCANCODE_BACKSLASH;
    case Keys::OemQuotes:       return SDL_SCANCODE_APOSTROPHE;
    case Keys::Pause:           return SDL_SCANCODE_PAUSE;
    default: return SDL_SCANCODE_UNKNOWN;
    }
}
} // namespace Microsoft::Xna::Framework::Input

using Microsoft::Xna::Framework::Input::KeyboardState;
bool KeyboardState::IsKeyDown(Keys key) const {
    SDL_Scancode sc = XNAKeyToScancode(key);
    if (sc == SDL_SCANCODE_UNKNOWN || sc >= SDL_SCANCODE_COUNT) return false;
    return sdlKeys[sc];
}

// ============================================================================
// GamePad
// ============================================================================
GamePadState GamePad::GetState(int playerIndex) {
    GamePadState gs;
    SDL_Gamepad* pad = SDL_GetGamepadFromPlayerIndex(playerIndex);
    gs.IsConnected = (pad != nullptr);
    gs._pad = pad;
    return gs;
}

bool GamePadState::IsButtonDown(Buttons button) const {
    if (!_pad) return false;
    switch (button) {
    case Buttons::A:            return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_SOUTH);
    case Buttons::B:            return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_EAST);
    case Buttons::X:            return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_WEST);
    case Buttons::Y:            return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_NORTH);
    case Buttons::Start:        return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_START);
    case Buttons::Back:         return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_BACK);
    case Buttons::DPadUp:       return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    case Buttons::DPadDown:     return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    case Buttons::DPadLeft:     return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    case Buttons::DPadRight:    return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    case Buttons::LeftShoulder: return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    case Buttons::RightShoulder:return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    case Buttons::LeftStick:    return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_LEFT_STICK);
    case Buttons::RightStick:   return SDL_GetGamepadButton(_pad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
    case Buttons::LeftTrigger:  return SDL_GetGamepadAxis(_pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER)  > 8000;
    case Buttons::RightTrigger: return SDL_GetGamepadAxis(_pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 8000;
    case Buttons::LeftThumbstickLeft:  return SDL_GetGamepadAxis(_pad, SDL_GAMEPAD_AXIS_LEFTX)  < -8000;
    case Buttons::LeftThumbstickRight: return SDL_GetGamepadAxis(_pad, SDL_GAMEPAD_AXIS_LEFTX)  >  8000;
    case Buttons::LeftThumbstickUp:    return SDL_GetGamepadAxis(_pad, SDL_GAMEPAD_AXIS_LEFTY)  < -8000;
    case Buttons::LeftThumbstickDown:  return SDL_GetGamepadAxis(_pad, SDL_GAMEPAD_AXIS_LEFTY)  >  8000;
    default: return false;
    }
}

// ============================================================================
// SoundEffectInstance
// ============================================================================
using Microsoft::Xna::Framework::Audio::SoundEffectInstance;
using Microsoft::Xna::Framework::Audio::SoundState;

void SoundEffectInstance::Play() {
    if (!_stream || _pcmData.empty()) return;
    if (State == SoundState::Playing) {
        // Restart: reset stream
        SDL_FlushAudioStream(_stream);
        SDL_SetAudioStreamGetCallback(_stream, nullptr, nullptr);
    }
    SDL_PutAudioStreamData(_stream, _pcmData.data(), (int)_pcmData.size());
    SDL_SetAudioStreamGain(_stream, Volume);
    if (IsLooped) {
        // Set a callback to re-queue PCM data when the stream runs low
        SDL_SetAudioStreamGetCallback(_stream,
            [](void* ud, SDL_AudioStream* s, int /*additional*/, int /*total*/) {
                auto* inst = static_cast<SoundEffectInstance*>(ud);
                SDL_PutAudioStreamData(s, inst->_pcmData.data(), (int)inst->_pcmData.size());
            }, this);
    }
    if (SDL3Context::AudioDevice) {
        SDL_BindAudioStream(SDL3Context::AudioDevice, _stream);
    }
    State = SoundState::Playing;
}

void SoundEffectInstance::Pause() {
    if (_stream && State == SoundState::Playing) {
        SDL_UnbindAudioStream(_stream);
        State = SoundState::Paused;
    }
}

void SoundEffectInstance::Resume() {
    if (_stream && State == SoundState::Paused) {
        SDL_SetAudioStreamGain(_stream, Volume);
        if (SDL3Context::AudioDevice)
            SDL_BindAudioStream(SDL3Context::AudioDevice, _stream);
        State = SoundState::Playing;
    }
}

void SoundEffectInstance::Stop(bool /*immediate*/) {
    if (_stream) {
        SDL_SetAudioStreamGetCallback(_stream, nullptr, nullptr);
        SDL_UnbindAudioStream(_stream);
        SDL_FlushAudioStream(_stream);
    }
    State = SoundState::Stopped;
}

void SoundEffectInstance::Dispose() {
    if (_stream) {
        SDL_SetAudioStreamGetCallback(_stream, nullptr, nullptr);
        SDL_UnbindAudioStream(_stream);
        SDL_DestroyAudioStream(_stream);
        _stream = nullptr;
    }
    _pcmData.clear();
    State = SoundState::Stopped;
}
