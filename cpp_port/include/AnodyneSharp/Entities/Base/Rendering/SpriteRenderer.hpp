#pragma once
#include "AnodyneSharp/Common.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Drawing/Spritesheet/Anim.hpp"
#include "AnodyneSharp/Drawing/Spritesheet/Spritesheet.hpp"
#include "XNA/Graphics.hpp"

namespace AnodyneSharp::Entities::Base::Rendering {

class Entity; // forward

class ILayerType {
public:
    virtual ~ILayerType() = default;
    virtual float Z() const = 0;
};

class NonEntityLayer : public ILayerType {
public:
    static NonEntityLayer Zero;
    Drawing::DrawOrder layer;
    NonEntityLayer(Drawing::DrawOrder l) : layer(l) {}
    float Z() const override;
};

// Forward declaration
class Entity;

class Layer : public ILayerType {
public:
    Drawing::DrawOrder layer;
    void* parent; // Entity* (avoid circular dependency)
    Layer(Drawing::DrawOrder l, void* p) : layer(l), parent(p) {}
    float Z() const override;
};

class RefLayer : public ILayerType {
public:
    ILayerType* parentLayer;
    int LayerOffset;
    RefLayer(ILayerType* parent, int offset) : parentLayer(parent), LayerOffset(offset) {}
    float Z() const override { return parentLayer->Z() - LayerOffset * 0.0001f; }
};

struct RenderProperties {
    Vector2 Position = {0,0};
    float scale = 1.f;
    int   y_push = 0;
    float rotation = 0.f;
    float opacity = 1.f;
    SpriteEffects flip = SpriteEffects::None;
};

class ISpriteRenderer {
public:
    virtual ~ISpriteRenderer() = default;
    virtual bool AnimFinished() const = 0;
    virtual Color& GetColor() = 0;
    virtual void   SetColor(const Color& c) = 0;
    virtual const std::string& CurAnimName() const = 0;
    virtual int Frame() const = 0;
    virtual int FrameIndex() const = 0;
    virtual int Height() const = 0;
    virtual int Width() const = 0;
    virtual ILayerType* GetLayer() const = 0;
    virtual void        SetLayer(ILayerType* layer) = 0;

    virtual void Draw(SpriteBatch& batch, const Vector2& position, float scale,
                      int y_push, float rotation, float opacity, SpriteEffects flip) = 0;
    virtual void SetFrame(int index) = 0;
    virtual bool PlayAnim(const std::string& name, bool force = false, std::optional<int> newFramerate = std::nullopt) = 0;
    virtual bool SetTexture(const std::string& textureName, int width, int height, bool ignoreChaos, bool allowFailure) = 0;
    virtual void Update() = 0;
};

class AnimatedSpriteRenderer : public ISpriteRenderer {
public:
    AnimatedSpriteRenderer(const std::string& textureName, int frameWidth, int frameHeight,
                           ILayerType* layer, std::vector<Anim> animations);
    AnimatedSpriteRenderer(const std::string& textureName, int frameWidth, int frameHeight,
                           std::vector<Anim> animations);
    // variadic constructor matching C# params Anim[]
    template<typename... As,
             std::enable_if_t<(sizeof...(As) > 0) &&
                              !(sizeof...(As) == 1 && (std::is_same_v<std::decay_t<As>, std::vector<Anim>> || ...)) &&
                              (std::is_convertible_v<std::decay_t<As>, Anim> && ...), int> = 0>
    AnimatedSpriteRenderer(const std::string& textureName, int frameWidth, int frameHeight, As... as)
        : AnimatedSpriteRenderer(textureName, frameWidth, frameHeight,
                                  makeAnimVec(std::move(as)...)) {}

    template<typename... As>
    static std::vector<Anim> makeAnimVec(As... as) {
        using expander = int[];
        std::vector<Anim> v;
        v.reserve(sizeof...(as));
        (void)expander{0, (v.push_back(static_cast<Anim>(std::move(as))), 0)...};
        return v;
    }

    bool AnimFinished() const override { return _curAnim ? _curAnim->Finished() : true; }
    Color& GetColor() override { return _color; }
    void   SetColor(const Color& c) override { _color = c; }
    const std::string& CurAnimName() const override;
    int Frame()      const override { return _curAnim ? _curAnim->Frame() : 0; }
    int FrameIndex() const override { return _curAnim ? _curAnim->CurIndex() : 0; }
    int Height()     const override { return _sprite.Height; }
    int Width()      const override { return _sprite.Width;  }
    ILayerType* GetLayer() const override { return _layer; }
    void        SetLayer(ILayerType* l) override { _layer = l; }

    void Draw(SpriteBatch& batch, const Vector2& position, float scale,
              int y_push, float rotation, float opacity, SpriteEffects flip) override;
    void SetFrame(int index) override;
    bool PlayAnim(const std::string& name, bool force = false, std::optional<int> newFramerate = std::nullopt) override;
    bool SetTexture(const std::string& textureName, int width, int height, bool ignoreChaos, bool allowFailure) override;
    void Update() override;

private:
    std::string _textureName;
    Spritesheet _sprite;
    std::unordered_map<std::string, Anim> _animations;
    Anim* _curAnim = nullptr;
    Color _color = Color::White;
    ILayerType* _layer = nullptr;
};

class StaticSpriteRenderer : public ISpriteRenderer {
public:
    StaticSpriteRenderer(const std::string& textureName, int frameWidth, int frameHeight,
                         int frame = 0, ILayerType* layer = nullptr, bool ignoreChaos = false);

    bool AnimFinished() const override { return false; }
    Color& GetColor() override { return _color; }
    void   SetColor(const Color& c) override { _color = c; }
    const std::string& CurAnimName() const override { static const std::string s; return s; }
    int Frame()      const override { return _curFrame; }
    int FrameIndex() const override { return 0; }
    int Height()     const override { return _sprite.Height; }
    int Width()      const override { return _sprite.Width;  }
    ILayerType* GetLayer() const override { return _layer; }
    void        SetLayer(ILayerType* l) override { _layer = l; }

    void Draw(SpriteBatch& batch, const Vector2& position, float scale,
              int y_push, float rotation, float opacity, SpriteEffects flip) override;
    void SetFrame(int index) override { _curFrame = index; }
    bool PlayAnim(const std::string& name, bool force = false, std::optional<int> newFramerate = std::nullopt) override { return false; }
    bool SetTexture(const std::string& textureName, int width, int height, bool ignoreChaos, bool allowFailure) override;
    void Update() override {}

private:
    std::string _textureName;
    Spritesheet _sprite;
    int _curFrame = 0;
    Color _color = Color::White;
    ILayerType* _layer = nullptr;
};

class SolidColorRenderer : public ISpriteRenderer {
public:
    SolidColorRenderer(Color color, int height, int width)
        : _height(height), _width(width), _color(color) {}

    bool AnimFinished() const override { return true; }
    Color& GetColor() override { return _color; }
    void   SetColor(const Color& c) override { _color = c; }
    const std::string& CurAnimName() const override { static const std::string s; return s; }
    int Frame()      const override { return 0; }
    int FrameIndex() const override { return 0; }
    int Height()     const override { return _height; }
    int Width()      const override { return _width; }
    ILayerType* GetLayer() const override { return _layer; }
    void        SetLayer(ILayerType* l) override { _layer = l; }

    void Draw(SpriteBatch& batch, const Vector2& position, float scale,
              int y_push, float rotation, float opacity, SpriteEffects flip) override;
    void SetFrame(int index) override {}
    bool PlayAnim(const std::string& name, bool force = false, std::optional<int> newFramerate = std::nullopt) override { return false; }
    bool SetTexture(const std::string& textureName, int width, int height, bool ignoreChaos, bool allowFailure) override { return false; }
    void Update() override {}

    int _height, _width;
    Color _color;
    ILayerType* _layer = nullptr;
};

// CompositeSpriteRenderer stub
class CompositeSpriteRenderer : public ISpriteRenderer {
public:
    // Contains multiple renderers
    std::vector<std::unique_ptr<ISpriteRenderer>> renderers;
    int _width = 0, _height = 0;
    Color _color = Color::White;
    ILayerType* _layer = nullptr;
    std::string _curAnim;

    CompositeSpriteRenderer(int w, int h) : _width(w), _height(h) {}
    void Add(std::unique_ptr<ISpriteRenderer> r) { renderers.push_back(std::move(r)); }

    bool AnimFinished() const override {
        for (auto& r : renderers) if (!r->AnimFinished()) return false;
        return true;
    }
    Color& GetColor() override { return _color; }
    void   SetColor(const Color& c) override { _color = c; for(auto& r:renderers) r->SetColor(c); }
    const std::string& CurAnimName() const override { return _curAnim; }
    int Frame()      const override { return renderers.empty() ? 0 : renderers[0]->Frame(); }
    int FrameIndex() const override { return renderers.empty() ? 0 : renderers[0]->FrameIndex(); }
    int Height()     const override { return _height; }
    int Width()      const override { return _width; }
    ILayerType* GetLayer() const override { return _layer; }
    void        SetLayer(ILayerType* l) override { _layer = l; for(auto& r:renderers) r->SetLayer(l); }

    void Draw(SpriteBatch& batch, const Vector2& position, float scale,
              int y_push, float rotation, float opacity, SpriteEffects flip) override {
        for (auto& r : renderers) r->Draw(batch, position, scale, y_push, rotation, opacity, flip);
    }
    void SetFrame(int index) override { for(auto& r:renderers) r->SetFrame(index); }
    bool PlayAnim(const std::string& name, bool force = false, std::optional<int> newFramerate = std::nullopt) override {
        bool result = false;
        for (auto& r : renderers) result |= r->PlayAnim(name, force, newFramerate);
        _curAnim = name;
        return result;
    }
    bool SetTexture(const std::string& textureName, int width, int height, bool ignoreChaos, bool allowFailure) override { return false; }
    void Update() override { for(auto& r : renderers) r->Update(); }
};

} // namespace AnodyneSharp::Entities::Base::Rendering

using AnodyneSharp::Entities::Base::Rendering::ISpriteRenderer;
using AnodyneSharp::Entities::Base::Rendering::ILayerType;
using AnodyneSharp::Entities::Base::Rendering::Layer;
using AnodyneSharp::Entities::Base::Rendering::NonEntityLayer;
using AnodyneSharp::Entities::Base::Rendering::RefLayer;
using AnodyneSharp::Entities::Base::Rendering::RenderProperties;
using AnodyneSharp::Entities::Base::Rendering::AnimatedSpriteRenderer;
using AnodyneSharp::Entities::Base::Rendering::StaticSpriteRenderer;
using AnodyneSharp::Entities::Base::Rendering::SolidColorRenderer;
using AnodyneSharp::Entities::Base::Rendering::CompositeSpriteRenderer;
