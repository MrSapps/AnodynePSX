#pragma once
#include "AnodyneSharp/Common.hpp"
#include "XNA/Graphics.hpp"
#include "XNA/Audio.hpp"

namespace AnodyneSharp::Resources {

// Forward
struct SFXLimiter;

class TextureHandle {
public:
    Texture2D* tex = nullptr;
    int Width  = 0;
    int Height = 0;

    TextureHandle() = default;
    TextureHandle(Texture2D* baseTex) : tex(baseTex),
        Width(baseTex ? baseTex->Width : 0),
        Height(baseTex ? baseTex->Height : 0) {}

    Texture2D* Tex() const { return tex; }
    void SetTex(Texture2D* t) { tex = t; }
};

class ResourceManager {
public:
    static std::string BaseDir;

    static bool LoadResources(ContentManager& content);
    static Texture2D*           GetTexture(const std::string& textureName, bool allowUnknown = false);
    static TextureHandle*       GetTexHandle(const std::string& texName, bool ignoreChaos = false, bool allowUnknown = false);
    static void                 ReloadRandomizableTextures();
    static std::string          GetMusicPath(const std::string& musicName);
    static std::string          GetAmbiencePath(const std::string& ambienceName);
    static SoundEffectInstance* GetSFX(const std::string& sfxName);

private:
    static std::unordered_map<std::string, Texture2D*>     _textures;
    static std::unordered_map<std::string, TextureHandle*> _randomizeableHandles;
    static std::unordered_map<std::string, std::string>    _music;
    static std::unordered_map<std::string, std::string>    _ambience;
    static std::unordered_map<std::string, std::unique_ptr<SFXLimiter>> _sfx;

    static void LoadTextures(ContentManager& content);
    static void LoadMusic(ContentManager& content);
    static void LoadAmbience(ContentManager& content);
    static void LoadSFX(ContentManager& content);
};

// SFXLimiter — wraps a pool of SoundEffectInstances
struct SFXLimiter {
    std::vector<std::unique_ptr<SoundEffectInstance>> pool;
    // Construct from a single pre-loaded instance
    SFXLimiter(std::unique_ptr<SoundEffectInstance> inst) {
        if (inst) pool.push_back(std::move(inst));
    }
    SoundEffectInstance* Get() {
        for (auto& e : pool)
            if (e->State == SoundState::Stopped) return e.get();
        return nullptr;
    }
};

} // namespace AnodyneSharp::Resources

using AnodyneSharp::Resources::ResourceManager;
using AnodyneSharp::Resources::TextureHandle;
using AnodyneSharp::Resources::SFXLimiter;
