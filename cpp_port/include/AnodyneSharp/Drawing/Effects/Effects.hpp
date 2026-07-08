#pragma once
#include "AnodyneSharp/Drawing/Effects/IFullScreenEffect.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <functional>

namespace AnodyneSharp::UI {
    class UIEntity;
    class UILabel;
}

namespace AnodyneSharp::Drawing::Effects {

// BlendEffect (blends overlay texture using shader) — renamed from 'Blend' to avoid XNA Blend enum conflict
class BlendEffect : public IFullScreenEffect {
public:
    bool Active() const override { return !_bakedPixels.empty(); }
    void Deactivate() override { tex = nullptr; hard_light = false; _bakedPixels.clear(); }
    void Load(ContentManager& content, GraphicsDevice& /*gd*/) override { /* stub */ }
    void Render(SpriteBatch& batch, Texture2D& screen) override { /* stub */ }
    void Update() override {}
    void SetTex(const std::string& texName);

    // Software blend access
    bool IsHardLight() const { return hard_light; }
    const std::vector<uint8_t>& GetBakedPixels() const { return _bakedPixels; }

protected:
    Texture2D* tex = nullptr;
    bool hard_light = false;
    float cutoff = 0.f;
    std::vector<uint8_t> _bakedPixels; // ARGB8888 scaled to 160×180
};

// Darkness effect
class Darkness : public IFullScreenEffect {
public:
    float Alpha = 0.f;
    bool Active() const override { return Alpha > 0.f; }
    void Deactivate() override { Alpha = 0.f; }
    void Load(ContentManager& /*content*/, GraphicsDevice& /*gd*/) override {}
    void Render(SpriteBatch& /*batch*/, Texture2D& /*screen*/) override {}
    void Update() override {
        if (_target >= 0.f && Alpha != _target) {
            Alpha += (_target - Alpha) * 0.05f;
            if (std::abs(Alpha - _target) < 0.01f) Alpha = _target;
        }
    }
    void ForceAlpha(float a)  { Alpha = a; _target = -1.f; }
    void TargetAlpha(float a) { _target = a; }
    void SetTex(const std::string& /*texName*/) {}  // rendering stub
private:
    float _target = -1.f;
};

// Flash effect
class FlashEffect : public IFullScreenEffect {
public:
    bool Active() const override { return _alpha > 0.f; }
    void Deactivate() override { _alpha = 0.f; }
    void Load(ContentManager& /*content*/, GraphicsDevice& /*gd*/) override {}
    void Render(SpriteBatch& /*batch*/, Texture2D& /*screen*/) override {}
    
    void Update() override;
    void Flash(float duration, Color color = Color::White, std::function<void()> onFull  = nullptr);

    float GetAlpha() const { return _alpha; }

    Color GetFlashColor() const {
        return Color{_color.R, _color.G, _color.B, (unsigned char)(_alpha * 255.f)};
    }

    void ForceAlpha(float a)
    {
        // TODO
        //target_alpha = alpha = a;
    }
private:
    float _duration = 0.3f;
    float _alpha    = 0.f;
    bool _easing   = false;
    Color _color    = Color::White;
    std::function<void()> _onFull;
};

// Wave effect
class Wave : public IFullScreenEffect {
public:
    bool Active() const override { return _active; }
    void Deactivate() override { _active = false; }
    void Load(ContentManager& /*c*/, GraphicsDevice& /*gd*/) override {}
    void Render(SpriteBatch& /*b*/, Texture2D& /*s*/) override {}
    void Update() override {
        if (!_active) return;
        _timer += AnodyneSharp::GameTimes::DeltaTime() * 0.5f;
        if (_timer >= 1.f) _timer -= 1.f;
    }
    void Activate() { _active = true; }
    int GetPhase() const { return (int)(_timer * 180.f); }
private:
    bool  _active = false;
    float _timer  = 0.f;
};

// Grayscale effect
class GrayScale : public IFullScreenEffect {
public:
    bool Active() const override { return _active; }
    void Deactivate() override { _active = false; }
    void Load(ContentManager& content, GraphicsDevice& gd) override {}
    void Render(SpriteBatch& batch, Texture2D& screen) override {}
    void Update() override {}
    void Activate() { _active = true; }
private:
    bool _active = false;
};

// Screen shake effect
class ScreenShake : public IFullScreenEffect {
public:
    Vector2 Offset = {0,0};  // applied to camera each frame

    bool Active() const override { return _timer > 0.f; }
    void Deactivate() override { _timer = 0.f; Offset = {0,0}; }
    void Load(ContentManager& /*content*/, GraphicsDevice& /*gd*/) override {}
    void Render(SpriteBatch& /*batch*/, Texture2D& /*screen*/) override {}

    void Update() override {
        if (_timer <= 0.f) { Offset = {0,0}; return; }
        _timer -= AnodyneSharp::GameTimes::DeltaTime();
        _shakeUpdate += AnodyneSharp::GameTimes::DeltaTime();
        if (_shakeUpdate < 1.f / 30.f) return;
        _shakeUpdate = 0.f;
        float r  = (float)std::rand() / (float)RAND_MAX;
        float r2 = (float)std::rand() / (float)RAND_MAX;
        Offset.X = (r  * 2.f - 1.f) * _intensity;
        Offset.Y = (r2 * 2.f - 1.f) * _intensity;
    }

    void Shake(float intensity, float time) {
        if (!Enabled) return;
        _timer     = time;
        _intensity = intensity;
        Offset     = {0,0};
    }
    static bool Enabled;  // synced from settings.screenshake in AnodyneGame::Update
private:
    float _timer       = 0.f;
    float _intensity   = 0.f;
    float _shakeUpdate = 0.f;
};

// Static noise effect
class Static : public IFullScreenEffect {
public:
    bool Active() const override { return _active; }
    void Deactivate() override { _active = false; }
    void Load(ContentManager& /*c*/, GraphicsDevice& /*gd*/) override {}
    void Render(SpriteBatch& /*b*/, Texture2D& /*s*/) override {}
    void Update() override {
        if (!_active) return;
        _staticTimer += AnodyneSharp::GameTimes::DeltaTime();
        if (_staticTimer >= 1.f / 8.f) {
            _staticTimer = 0.f;
            _staticStep  = (_staticStep + 1) % 4;
        }
    }
    void Activate() { _active = true; }
    int GetStep() const { return _staticStep; }
private:
    bool  _active      = false;
    float _staticTimer = 0.f;
    int   _staticStep  = 0;
};

// Pixelate effect
class Pixelate : public IFullScreenEffect {
public:
    bool Active() const override { return _active; }
    void Deactivate() override { _active = false; _pixelSize = 1.f; }
    void Load(ContentManager& /*c*/, GraphicsDevice& /*gd*/) override {}
    void Render(SpriteBatch& /*b*/, Texture2D& /*s*/) override {}
    void Update() override {}
    void Activate(float maxPixelSize) { _active = true; _pixelSize = maxPixelSize; }
    void SetPixelation(float v) { _pixelSize = v; _active = (v > 1.f); }
    void AddPixelation(float speedPerSec) {
        _pixelSize += speedPerSec * AnodyneSharp::GameTimes::DeltaTime();
        if (_pixelSize < 1.f) { _pixelSize = 1.f; _active = false; }
        else { _active = true; }
    }
    int GetPixelSize() const { return (int)_pixelSize; }
private:
    bool  _active       = false;
    float _pixelSize    = 1.f;
    float _maxPixelSize = 1.f;
};

// FG Blend
class FG_Blend : public BlendEffect {
public:
    FG_Blend() = default;
};

// Title Screen Overlay
class TitleScreenOverlay : public IFullScreenEffect {
public:
    bool Active() const override { return _active; }
    void Deactivate() override { _active = false; }
    void Load(ContentManager& content, GraphicsDevice& gd) override {}
    void Render(SpriteBatch& batch, Texture2D& screen) override {}
    void Update() override {}
    void Activate() { _active = true; }
    Texture2D* Darkness = nullptr;  // set by TitleState
    std::vector<AnodyneSharp::UI::UIEntity*> Entities;  // set by TitleState
    std::vector<AnodyneSharp::UI::UILabel*> Labels;     // set by TitleState

    void ForceAlpha(float a)
    {
        // TODO
    }

private:
    bool _active = false;
};

// Glitch effect
class Glitch : public IFullScreenEffect {
public:
    struct GlitchRect {
        int srcX, srcY, srcW, srcH, dstX, dstY;
        void Recalc() {
            srcX = std::rand() % 160; srcY = std::rand() % 180;
            srcW = 16 + std::rand() % 17; srcH = 16 + std::rand() % 17;
            dstX = std::rand() % 160; dstY = std::rand() % 180;
        }
        GlitchRect() { Recalc(); }
    };
    bool Active() const override { return _active; }
    void Deactivate() override { _active = false; }
    void Load(ContentManager& /*c*/, GraphicsDevice& /*gd*/) override {}
    void Render(SpriteBatch& /*b*/, Texture2D& /*s*/) override {}
    void Update() override {
        if (!_active) return;
        _timer += AnodyneSharp::GameTimes::DeltaTime();
        if (_timer >= 1.f / 3.f) {
            _timer -= 1.f / 3.f;
            for (auto& r : _rects) r.Recalc();
        }
    }
    void Activate() {
        _active = true;
        if (_rects.empty()) _rects.resize(50);
    }
    const std::vector<GlitchRect>& GetRects() const { return _rects; }
private:
    bool  _active = false;
    float _timer  = 0.f;
    std::vector<GlitchRect> _rects;
};

// FadeEffect
class FadeEffect : public IFullScreenEffect {
public:
    float alpha = 0.f;
    bool Active() const override { return _active; }
    void Deactivate() override { _active = false; alpha = 0.f; }
    void Load(ContentManager& content, GraphicsDevice& gd) override {}
    void Render(SpriteBatch& batch, Texture2D& screen) override {}
    void Update() override {}
    void Fade(float duration, bool fadeIn = true) {}
    void ForceAlpha(float a) { alpha = a; _active = (a > 0.f); }
    // rate = fractional change per second (matches C# speed * DeltaTime)
    void ChangeAlpha(float rate) {
        alpha += rate * AnodyneSharp::GameTimes::DeltaTime();
        if (alpha < 0.f) alpha = 0.f;
        if (alpha > 1.f) alpha = 1.f;
        _active = (alpha > 0.f);
    }
private:
    bool _active = false;
    float _timer = 0.f;
    float _duration = 0.f;
    bool _fadeIn = true;
};

} // namespace AnodyneSharp::Drawing::Effects

using AnodyneSharp::Drawing::Effects::IFullScreenEffect;
using AnodyneSharp::Drawing::Effects::Darkness;
using AnodyneSharp::Drawing::Effects::FlashEffect;
using AnodyneSharp::Drawing::Effects::Wave;
using AnodyneSharp::Drawing::Effects::GrayScale;
using AnodyneSharp::Drawing::Effects::ScreenShake;
using AnodyneSharp::Drawing::Effects::Static;
using AnodyneSharp::Drawing::Effects::Pixelate;
using AnodyneSharp::Drawing::Effects::BlendEffect;
using AnodyneSharp::Drawing::Effects::FG_Blend;
using AnodyneSharp::Drawing::Effects::TitleScreenOverlay;
using AnodyneSharp::Drawing::Effects::Glitch;
using AnodyneSharp::Drawing::Effects::FadeEffect;
