#pragma once
#include <SDL3/SDL.h>

// Global SDL3 runtime context — set once at startup by Game::Run()
namespace SDL3Context {
    extern SDL_Window*       Window;
    extern SDL_Renderer*     Renderer;
    extern SDL_AudioDeviceID AudioDevice;
    extern SDL_AudioSpec     AudioSpec;   // actual device output spec

    // Offscreen render targets for post-processing effects
    extern SDL_Texture*      OffscreenTarget;  // ARGB8888 TARGET  160×180 — all scene draws go here
    extern SDL_Texture*      EffectTexture;    // ARGB8888 STREAMING 160×180 — CPU pixel effects output

    // Scale: game renders at GAME_W x GAME_H, window is scale× that
    constexpr int GAME_W = 160;
    constexpr int GAME_H = 180;
    constexpr int SCALE  = 4;
    constexpr int WIN_W  = GAME_W * SCALE;
    constexpr int WIN_H  = GAME_H * SCALE;
}
