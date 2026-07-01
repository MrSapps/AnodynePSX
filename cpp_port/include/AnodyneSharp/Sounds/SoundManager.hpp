#pragma once
#include "AnodyneSharp/Common.hpp"
#include "XNA/Audio.hpp"

namespace AnodyneSharp::Sounds {

class SoundManager {
public:
    static std::string CurrentSongName;
    static bool IsPlayingSong() { return !CurrentSongName.empty(); }

    static bool  PlaySong(const std::string& name, float volume = 1.f);
    static void  SetMasterVolume(float volume);
    static float GetMasterVolume();
    static void  SetSongVolume(float volume);
    static void  SetSongVolume();
    static float GetVolume();
    static bool  StopSong();
    static void  PlayAmbience(const std::string& name, float volume = 1.f);
    static void  SetAmbienceVolume(float volume);

    template<typename... Args>
    static SoundEffectInstance* PlaySoundEffect(Args&&... names) {
        std::vector<std::string> v = {std::forward<Args>(names)...};
        return PlaySoundEffectImpl(v);
    }
    static SoundEffectInstance* PlaySoundEffectImpl(const std::vector<std::string>& names);
    static void PlayPitchedSoundEffect(const std::string& name, float pitch, float volume = 1.f);

private:
    static float _masterVolume;
    static float _currentVolume;
    static float _ambienceVolume;
    static SoundEffectInstance* CreateSoundInstance(SoundEffectInstance* sfx,
                                                    float volume = 1.f, float pitch = 0.f);
};

} // namespace AnodyneSharp::Sounds

using AnodyneSharp::Sounds::SoundManager;
