#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "SDL3Context.hpp"
#include <SDL3/SDL.h>
#include <filesystem>
#include <fstream>

// stb_image for PNG loading
#ifdef HAS_STB_IMAGE
#  define STB_IMAGE_IMPLEMENTATION
#  include "thirdparty/stb_image.h"
#endif

// dr_mp3 for MP3 loading
#ifdef HAS_DR_MP3
#  define DR_MP3_IMPLEMENTATION
#  include "thirdparty/dr_mp3.h"
#endif

namespace fs = std::filesystem;

namespace AnodyneSharp::Resources {

std::string ResourceManager::BaseDir = ".";
std::unordered_map<std::string, Texture2D*>     ResourceManager::_textures;
std::unordered_map<std::string, TextureHandle*> ResourceManager::_randomizeableHandles;
std::unordered_map<std::string, std::string>    ResourceManager::_music;
std::unordered_map<std::string, std::string>    ResourceManager::_ambience;
std::unordered_map<std::string, std::unique_ptr<SFXLimiter>> ResourceManager::_sfx;

bool ResourceManager::LoadResources(ContentManager& content) {
    LoadTextures(content);
    LoadMusic(content);
    LoadAmbience(content);
    LoadSFX(content);
    return true;
}

Texture2D* ResourceManager::GetTexture(const std::string& textureName, bool /*allowUnknown*/) {
    auto it = _textures.find(textureName);
    return it != _textures.end() ? it->second : nullptr;
}

TextureHandle* ResourceManager::GetTexHandle(const std::string& texName, bool ignoreChaos, bool /*allowUnknown*/) {
    if (!ignoreChaos) {
        auto it = _randomizeableHandles.find(texName);
        if (it != _randomizeableHandles.end()) return it->second;
    }
    auto texIt = _textures.find(texName);
    if (texIt == _textures.end()) return nullptr;
    auto* handle = new TextureHandle(texIt->second);
    if (!ignoreChaos) _randomizeableHandles[texName] = handle;
    return handle;
}

void ResourceManager::ReloadRandomizableTextures() {}

std::string ResourceManager::GetMusicPath(const std::string& musicName) {
    auto it = _music.find(musicName);
    return it != _music.end() ? it->second : "";
}

std::string ResourceManager::GetAmbiencePath(const std::string& ambienceName) {
    auto it = _ambience.find(ambienceName);
    return it != _ambience.end() ? it->second : "";
}

SoundEffectInstance* ResourceManager::GetSFX(const std::string& sfxName) {
    auto it = _sfx.find(sfxName);
    return it != _sfx.end() ? it->second->Get() : nullptr;
}

// ---- Texture loading -------------------------------------------------------

static Texture2D* LoadPNG(const std::string& path) {
    SDL_Renderer* ren = SDL3Context::Renderer;
    if (!ren) return nullptr;

#ifdef HAS_STB_IMAGE
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) return nullptr;

    SDL_Surface* surf = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, data, w * 4);
    if (!surf) { stbi_image_free(data); return nullptr; }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_DestroySurface(surf);
    if (!tex) { stbi_image_free(data); return nullptr; }

    auto* t = new Texture2D();
    t->AttachSDL(tex, w, h);
    // Cache raw RGBA bytes so effects (e.g. BlendEffect) can read pixel data on the CPU
    t->cpuPixels.assign(data, data + w * h * 4);
    t->cpuWidth  = w;
    t->cpuHeight = h;
    stbi_image_free(data);
    return t;
#else
    // Fallback: create a 16×16 magenta "missing" texture
    SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                                          SDL_TEXTUREACCESS_STATIC, 16, 16);
    if (!tex) return nullptr;
    std::vector<Uint32> pixels(16*16, 0xFF00FFFF);
    SDL_UpdateTexture(tex, nullptr, pixels.data(), 16 * 4);
    auto* t = new Texture2D();
    t->AttachSDL(tex, 16, 16);
    return t;
#endif
}

void ResourceManager::LoadTextures(ContentManager& /*content*/) {
    // Scan the textures directory relative to BaseDir
    std::string texDir = BaseDir + "/Content/textures";
    if (!fs::exists(texDir)) {
        SDL_Log("ResourceManager: textures dir not found: %s", texDir.c_str());
        return;
    }
    for (auto& entry : fs::recursive_directory_iterator(texDir)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        // lower-case extension comparison
        std::string extL = ext;
        for (auto& c : extL) c = (char)std::tolower((unsigned char)c);
        if (extL != ".png" && extL != ".jpg" && extL != ".jpeg") continue;

        std::string key = entry.path().stem().string();
        if (_textures.count(key)) continue;  // already loaded

        Texture2D* t = LoadPNG(entry.path().string());
        if (t) _textures[key] = t;
    }
    SDL_Log("ResourceManager: loaded %zu textures", _textures.size());
}

// ---- Music/Ambience (path registration only — audio streams created on demand) ----

void ResourceManager::LoadMusic(ContentManager& /*content*/) {
    std::string bgmDir = BaseDir + "/Content/bgm";
    if (!fs::exists(bgmDir)) return;
    for (auto& entry : fs::recursive_directory_iterator(bgmDir)) {
        if (!entry.is_regular_file()) continue;
        std::string key = entry.path().stem().string();
        _music[key] = entry.path().string();
    }
    SDL_Log("ResourceManager: registered %zu music tracks", _music.size());
}

void ResourceManager::LoadAmbience(ContentManager& /*content*/) {
    std::string ambDir = BaseDir + "/Content/ambience";
    if (!fs::exists(ambDir)) return;
    for (auto& entry : fs::recursive_directory_iterator(ambDir)) {
        if (!entry.is_regular_file()) continue;
        std::string key = entry.path().stem().string();
        _ambience[key] = entry.path().string();
    }
}

// ---- SFX loading -----------------------------------------------------------

static std::unique_ptr<SoundEffectInstance> LoadSFXFile(const std::string& path) {
    auto inst = std::make_unique<SoundEffectInstance>();

    // Try WAV first (SDL3 built-in)
    std::string extL = fs::path(path).extension().string();
    for (auto& c : extL) c = (char)std::tolower((unsigned char)c);

    if (extL == ".wav") {
        SDL_AudioSpec spec{};
        Uint8* buf = nullptr; Uint32 len = 0;
        if (!SDL_LoadWAV(path.c_str(), &spec, &buf, &len)) return nullptr;

        // Convert to device format
        SDL_AudioSpec devSpec = SDL3Context::AudioSpec;
        if (devSpec.freq == 0) devSpec = { SDL_AUDIO_F32LE, 2, 44100 };

        inst->_stream = SDL_CreateAudioStream(&spec, &devSpec);
        if (!inst->_stream) { SDL_free(buf); return nullptr; }
        inst->_spec = spec;
        inst->_pcmData.assign(buf, buf + len);
        SDL_free(buf);
        return inst;
    }

#ifdef HAS_DR_MP3
    if (extL == ".mp3") {
        drmp3_config cfg{};
        drmp3_uint64 frameCount = 0;
        float* pcm = drmp3_open_file_and_read_pcm_frames_f32(
            path.c_str(), &cfg, &frameCount, nullptr);
        if (!pcm) return nullptr;

        SDL_AudioSpec srcSpec{ SDL_AUDIO_F32LE, (int)cfg.channels, (int)cfg.sampleRate };
        SDL_AudioSpec devSpec = SDL3Context::AudioSpec;
        if (devSpec.freq == 0) devSpec = { SDL_AUDIO_F32LE, 2, 44100 };

        inst->_stream = SDL_CreateAudioStream(&srcSpec, &devSpec);
        if (!inst->_stream) { drmp3_free(pcm, nullptr); return nullptr; }
        inst->_spec = srcSpec;
        size_t byteLen = (size_t)frameCount * cfg.channels * sizeof(float);
        inst->_pcmData.assign((Uint8*)pcm, (Uint8*)pcm + byteLen);
        drmp3_free(pcm, nullptr);
        return inst;
    }
#endif

    return nullptr;  // unsupported format
}

void ResourceManager::LoadSFX(ContentManager& /*content*/) {
    std::string sfxDir = BaseDir + "/Content/sfx";
    if (!fs::exists(sfxDir)) return;
    for (auto& entry : fs::recursive_directory_iterator(sfxDir)) {
        if (!entry.is_regular_file()) continue;
        std::string key = entry.path().stem().string();
        if (_sfx.count(key)) continue;

        auto inst = LoadSFXFile(entry.path().string());
        if (inst) {
            _sfx[key] = std::make_unique<SFXLimiter>(std::move(inst));
        }
    }
    SDL_Log("ResourceManager: loaded %zu SFX", _sfx.size());
}

} // namespace AnodyneSharp::Resources
