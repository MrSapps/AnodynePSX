#pragma once
#include <string>
#include <SDL3/SDL.h>

namespace Microsoft { namespace Xna { namespace Framework { namespace Audio {

// ---------------------------------------------------------------
// SoundState
// ---------------------------------------------------------------
enum class SoundState {
    Playing,
    Paused,
    Stopped
};

// ---------------------------------------------------------------
// SoundEffectInstance — SDL3 audio stream
// ---------------------------------------------------------------
class SoundEffectInstance {
public:
    float Volume  = 1.0f;
    float Pitch   = 0.0f;
    float Pan     = 0.0f;
    bool  IsLooped = false;
    SoundState State = SoundState::Stopped;

    // SDL3 backing
    SDL_AudioStream* _stream = nullptr;
    std::vector<Uint8> _pcmData;   // full decoded audio
    SDL_AudioSpec     _spec{};

    void Play();
    void Pause();
    void Resume();
    void Stop(bool immediate = true);
    void Dispose();
    virtual ~SoundEffectInstance() { Dispose(); }
};

// ---------------------------------------------------------------
// SoundEffect (stub)
// ---------------------------------------------------------------
class SoundEffect {
public:
    float Duration = 0.f;

    void Play() {}
    void Play(float volume, float pitch, float pan) {}
    SoundEffectInstance* CreateInstance() { return new SoundEffectInstance(); }

    static float MasterVolume;
    virtual ~SoundEffect() = default;
};

// ---------------------------------------------------------------
// AudioListener (stub)
// ---------------------------------------------------------------
struct AudioListener {
    Microsoft::Xna::Framework::Vector3 Position;
    Microsoft::Xna::Framework::Vector3 Up;
    Microsoft::Xna::Framework::Vector3 Forward;
    Microsoft::Xna::Framework::Vector3 Velocity;
};

// ---------------------------------------------------------------
// AudioEmitter (stub)
// ---------------------------------------------------------------
struct AudioEmitter {
    Microsoft::Xna::Framework::Vector3 Position;
    Microsoft::Xna::Framework::Vector3 Up;
    Microsoft::Xna::Framework::Vector3 Forward;
    Microsoft::Xna::Framework::Vector3 Velocity;
    float DopplerScale = 1.0f;
};

}}}} // namespace Microsoft::Xna::Framework::Audio

using Microsoft::Xna::Framework::Audio::SoundState;
using Microsoft::Xna::Framework::Audio::SoundEffectInstance;
using Microsoft::Xna::Framework::Audio::SoundEffect;
