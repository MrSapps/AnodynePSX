#pragma once
#include "Framework.hpp"
#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace Microsoft { namespace Xna { namespace Framework { namespace Graphics {

// ---------------------------------------------------------------
// SpriteEffects
// ---------------------------------------------------------------
enum class SpriteEffects {
    None = 0,
    FlipHorizontally = 1,
    FlipVertically = 2
};

// ---------------------------------------------------------------
// SpriteSortMode
// ---------------------------------------------------------------
enum class SpriteSortMode {
    Deferred = 0,
    Immediate = 1,
    Texture = 2,
    BackToFront = 3,
    FrontToBack = 4
};

// ---------------------------------------------------------------
// GraphicsProfile
// ---------------------------------------------------------------
enum class GraphicsProfile {
    Reach = 0,
    HiDef = 1
};

// ---------------------------------------------------------------
// BlendFunction
// ---------------------------------------------------------------
enum class BlendFunction {
    Add = 0,
    Subtract,
    ReverseSubtract,
    Max,
    Min
};

// ---------------------------------------------------------------
// Blend
// ---------------------------------------------------------------
enum class Blend {
    One = 0,
    Zero,
    SourceColor,
    InverseSourceColor,
    SourceAlpha,
    InverseSourceAlpha,
    DestinationAlpha,
    InverseDestinationAlpha,
    DestinationColor,
    InverseDestinationColor,
    SourceAlphaSaturation,
    BlendFactor,
    InverseBlendFactor
};

// ---------------------------------------------------------------
// BlendState (stub)
// ---------------------------------------------------------------
struct BlendState {
    BlendFunction ColorBlendFunction = BlendFunction::Add;
    Blend ColorSourceBlend = Blend::One;
    Blend ColorDestinationBlend = Blend::Zero;
    BlendFunction AlphaBlendFunction = BlendFunction::Add;
    Blend AlphaSourceBlend = Blend::One;
    Blend AlphaDestinationBlend = Blend::Zero;

    static BlendState AlphaBlend;
    static BlendState Additive;
    static BlendState NonPremultiplied;
    static BlendState Opaque;
};

// ---------------------------------------------------------------
// SamplerState (stub)
// ---------------------------------------------------------------
struct SamplerState {
    static SamplerState PointClamp;
    static SamplerState PointWrap;
    static SamplerState LinearClamp;
    static SamplerState LinearWrap;
    static SamplerState AnisotropicClamp;
    static SamplerState AnisotropicWrap;
};

// ---------------------------------------------------------------
// EffectParameter (stub)
// ---------------------------------------------------------------
class EffectParameter {
public:
    void SetValue(bool v) {}
    void SetValue(int v) {}
    void SetValue(float v) {}
    void SetValue(const Vector2& v) {}
    void SetValue(const Vector3& v) {}
    void SetValue(const Vector4& v) {}
    void SetValue(const Matrix& v) {}
    void SetValue(struct Texture2D* v) {}
    void SetValue(const std::vector<float>& v) {}
};

// ---------------------------------------------------------------
// EffectTechnique (stub)
// ---------------------------------------------------------------
struct EffectTechnique {
    std::string Name;
};

// ---------------------------------------------------------------
// EffectTechniqueCollection (stub)
// ---------------------------------------------------------------
class EffectTechniqueCollection {
public:
    EffectTechnique& operator[](const std::string& name) { return _dummy; }
private:
    EffectTechnique _dummy;
};

// ---------------------------------------------------------------
// EffectParameterCollection (stub)
// ---------------------------------------------------------------
class EffectParameterCollection {
public:
    EffectParameter& operator[](const std::string& name) { return _dummy; }
private:
    EffectParameter _dummy;
};

// ---------------------------------------------------------------
// Effect (stub)
// ---------------------------------------------------------------
class Effect {
public:
    EffectParameterCollection Parameters;
    EffectTechniqueCollection Techniques;
    EffectTechnique* CurrentTechnique = nullptr;
    virtual ~Effect() = default;
};

// ---------------------------------------------------------------
// Texture2D
// ---------------------------------------------------------------
class Texture2D {
public:
    int Width = 0;
    int Height = 0;
    Rectangle Bounds;
    SDL_Texture* sdlTex = nullptr;  // SDL3 backing

    // Raw RGBA pixel data (stb_image byte order: R,G,B,A). Populated by ResourceManager.
    std::vector<uint8_t> cpuPixels;
    int cpuWidth  = 0;
    int cpuHeight = 0;

    Texture2D() = default;
    Texture2D(struct GraphicsDevice* dev, int w, int h);
    void AttachSDL(SDL_Texture* t, int w, int h);

    template<typename T>
    void SetData(const std::vector<T>& data) {}

    virtual ~Texture2D();
};

// ---------------------------------------------------------------
// RenderTargetBinding (stub)
// ---------------------------------------------------------------
struct RenderTargetBinding {
    Texture2D* RenderTarget = nullptr;
};

// ---------------------------------------------------------------
// RenderTarget2D — SDL_TEXTUREACCESS_TARGET texture
// ---------------------------------------------------------------
class RenderTarget2D : public Texture2D {
public:
    RenderTarget2D() = default;
    RenderTarget2D(struct GraphicsDevice* dev, int w, int h);
    virtual ~RenderTarget2D() = default;
};

// ---------------------------------------------------------------
// DisplayMode (stub)
// ---------------------------------------------------------------
struct DisplayMode {
    int Width = 1920;
    int Height = 1080;
};

// ---------------------------------------------------------------
// GraphicsAdapter (stub)
// ---------------------------------------------------------------
struct GraphicsAdapter {
    DisplayMode CurrentDisplayMode;
    static GraphicsAdapter DefaultAdapter;
    static GraphicsAdapter& GetDefaultAdapter() { return DefaultAdapter; }
};

// ---------------------------------------------------------------
// PresentationParameters (stub)
// ---------------------------------------------------------------
struct PresentationParameters {
    int BackBufferWidth = 160;
    int BackBufferHeight = 180;
};

// ---------------------------------------------------------------
// GraphicsDevice — wraps SDL_Renderer
// ---------------------------------------------------------------
class GraphicsDevice {
public:
    PresentationParameters PresentationParameters;
    SDL_Renderer* sdlRenderer = nullptr;  // set by Game::Run()
    SDL_Texture*  currentRT   = nullptr;  // current render target (null=screen)

    void Clear(const Color& color);
    void SetRenderTarget(RenderTarget2D* target);
    void SetRenderTargets(const std::vector<RenderTargetBinding>& targets) {}
    void SetRenderTargets(RenderTarget2D* t1, RenderTarget2D* t2) {}
    std::vector<RenderTargetBinding> GetRenderTargets() { return {}; }
    virtual ~GraphicsDevice() = default;
};

// ---------------------------------------------------------------
// SpriteBatch — collects draw calls, sorts by Z, flushes to SDL_Renderer
// ---------------------------------------------------------------
struct DrawCall {
    SDL_Texture* tex = nullptr;
    SDL_FRect src{}, dst{};
    SDL_FPoint origin{};
    double angleDeg = 0.0;
    SDL_FlipMode flip = SDL_FLIP_NONE;
    SDL_Color color{255,255,255,255};
    float z = 0.f;
};

class SpriteBatch {
public:
    GraphicsDevice* _gdev     = nullptr;
    SDL_Renderer*   _renderer = nullptr;
    std::vector<DrawCall> _calls;

    SpriteBatch() = default;
    SpriteBatch(class GraphicsDevice* dev);

    void Begin(SpriteSortMode sortMode = SpriteSortMode::Deferred,
               BlendState blendState = BlendState::AlphaBlend,
               SamplerState samplerState = SamplerState::PointClamp,
               Effect* effect = nullptr,
               Matrix* transformMatrix = nullptr) { _calls.clear(); }

    void Begin(SpriteSortMode sortMode,
               BlendState* blendState,
               SamplerState* samplerState = nullptr,
               Effect* effect = nullptr,
               Matrix* transformMatrix = nullptr) { _calls.clear(); }

    void Draw(Texture2D* texture, const Rectangle& dest, const Color& color);
    void Draw(Texture2D* texture, const Rectangle& dest, const Rectangle* sourceRect,
              const Color& color, float rotation = 0.f, const Vector2& origin = Vector2{},
              SpriteEffects effects = SpriteEffects::None, float layerDepth = 0.f);
    void Draw(Texture2D* texture, const Vector2& pos, const Color& color);
    void Draw(Texture2D* texture, const Vector2& pos, const Rectangle* sourceRect,
              const Color& color, float rotation = 0.f, const Vector2& origin = Vector2{},
              float scale = 1.f, SpriteEffects effects = SpriteEffects::None, float layerDepth = 0.f);

    void End();
    virtual ~SpriteBatch() = default;
};

// ---------------------------------------------------------------
// GraphicsDeviceManager (stub)
// ---------------------------------------------------------------
class GraphicsDeviceManager {
public:
    int PreferredBackBufferWidth = 160;
    int PreferredBackBufferHeight = 180;
    bool IsFullScreen = false;
    bool SynchronizeWithVerticalRetrace = true;
    GraphicsProfile GraphicsProfile = GraphicsProfile::HiDef;
    GraphicsDevice* GraphicsDevice = nullptr;

    GraphicsDeviceManager() = default;
    GraphicsDeviceManager(Game* game) {}
    void ApplyChanges() {}
};

}}}} // namespace Microsoft::Xna::Framework::Graphics

// Bring graphics types into scope
using Microsoft::Xna::Framework::Graphics::SpriteEffects;
using Microsoft::Xna::Framework::Graphics::SpriteSortMode;
using Microsoft::Xna::Framework::Graphics::GraphicsProfile;
using Microsoft::Xna::Framework::Graphics::BlendFunction;
using Microsoft::Xna::Framework::Graphics::Blend;
using Microsoft::Xna::Framework::Graphics::BlendState;
using Microsoft::Xna::Framework::Graphics::SamplerState;
using Microsoft::Xna::Framework::Graphics::Effect;
using Microsoft::Xna::Framework::Graphics::EffectParameter;
using Microsoft::Xna::Framework::Graphics::Texture2D;
using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
using Microsoft::Xna::Framework::Graphics::RenderTargetBinding;
using Microsoft::Xna::Framework::Graphics::GraphicsAdapter;
using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
using Microsoft::Xna::Framework::Graphics::SpriteBatch;
using Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager;
using Microsoft::Xna::Framework::Graphics::DisplayMode;
