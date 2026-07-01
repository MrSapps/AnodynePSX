#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include <algorithm>

namespace AnodyneSharp::Sounds {

std::string SoundManager::CurrentSongName;
float SoundManager::_masterVolume  = 1.f;
float SoundManager::_currentVolume = 1.f;
float SoundManager::_ambienceVolume = 1.f;

bool SoundManager::PlaySong(const std::string& name, float volume) {
    if (CurrentSongName == name) { SetSongVolume(volume); return false; }
    auto path = Resources::ResourceManager::GetMusicPath(name);
    if (!path.empty()) {
        CurrentSongName = name;
        SetSongVolume(volume);
        return true;
    }
    StopSong(); return false;
}

void SoundManager::SetMasterVolume(float v) {
    _masterVolume = std::clamp(v, 0.f, 1.f);
    SetSongVolume(_currentVolume);
}

float SoundManager::GetMasterVolume() { return _masterVolume; }

void SoundManager::SetSongVolume(float v) {
    _currentVolume = std::clamp(v, 0.f, 1.f);
    SetAmbienceVolume(_ambienceVolume);
}

void SoundManager::SetSongVolume() { SetSongVolume(1.f); }
float SoundManager::GetVolume()    { return _currentVolume; }

void SoundManager::SetAmbienceVolume(float v) {
    _ambienceVolume = std::clamp(v, 0.f, 1.f);
}

bool SoundManager::StopSong() {
    if (IsPlayingSong()) { CurrentSongName = ""; return true; }
    return false;
}

void SoundManager::PlayAmbience(const std::string& name, float volume) {
    SetAmbienceVolume(volume);
}

SoundEffectInstance* SoundManager::PlaySoundEffectImpl(const std::vector<std::string>& names) {
    for (auto& n : names) {
        auto* sfx = Resources::ResourceManager::GetSFX(n);
        if (sfx) return CreateSoundInstance(sfx);
    }
    return nullptr;
}

void SoundManager::PlayPitchedSoundEffect(const std::string& name, float pitch, float volume) {
    auto* sfx = Resources::ResourceManager::GetSFX(name);
    CreateSoundInstance(sfx, volume, pitch);
}

SoundEffectInstance* SoundManager::CreateSoundInstance(SoundEffectInstance* sfx, float volume, float pitch) {
    if (sfx) {
        sfx->Pitch  = pitch;
        sfx->Volume = volume * GlobalState::settings.sfx_volume_scale;
        sfx->Play();
    }
    return sfx;
}

} // namespace AnodyneSharp::Sounds
