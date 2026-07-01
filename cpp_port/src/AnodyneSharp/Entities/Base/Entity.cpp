#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/Rendering/SpriteRenderer.hpp"
#include "AnodyneSharp/Entities/Base/Shadow.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include <cmath>

namespace AnodyneSharp::Entities {

// Destructor defined here where Shadow is complete
Entity::~Entity() = default;

// --- Entity constructors ---

Entity::Entity(Vector2 pos, int w, int h)
    : GameObject(pos, w, h) { visible = false; }

Entity::Entity(Vector2 pos, Drawing::DrawOrder layer)
    : GameObject(pos) {
    _layerCache = std::make_unique<Base::Rendering::NonEntityLayer>(layer);
}

Entity::Entity(Vector2 pos, const std::string& textureName, int fw, int fh, Drawing::DrawOrder layer)
    : GameObject(pos, fw, fh) {
    auto l = std::make_unique<Base::Rendering::Layer>(layer, this);
    sprite = std::make_unique<Base::Rendering::StaticSpriteRenderer>(textureName, fw, fh, 0, l.release());
}

Entity::Entity(Vector2 pos, const std::string& textureName, int fw, int fh, Base::Rendering::ILayerType* layer)
    : GameObject(pos, fw, fh) {
    sprite = std::make_unique<Base::Rendering::StaticSpriteRenderer>(textureName, fw, fh, 0, layer);
}

Entity::Entity(Vector2 pos, std::unique_ptr<ISpriteRenderer> spr)
    : GameObject(pos, spr ? spr->Width() : 0, spr ? spr->Height() : 0) {
    sprite = std::move(spr);
}

Entity::Entity(Vector2 pos, std::unique_ptr<ISpriteRenderer> spr, Drawing::DrawOrder layer)
    : GameObject(pos, spr ? spr->Width() : 0, spr ? spr->Height() : 0) {
    sprite = std::move(spr);
    if (sprite) {
        auto l = std::make_unique<Base::Rendering::Layer>(layer, this);
        sprite->SetLayer(l.release());
    }
}

void Entity::set_layer(Drawing::DrawOrder value) {
    auto l = std::make_unique<Base::Rendering::Layer>(value, this);
    layer_def_set(l.release());
}

ILayerType* Entity::layer_def_get() const {
    if (sprite) return sprite->GetLayer();
    return _layerCache.get();
}

void Entity::layer_def_set(ILayerType* value) {
    if (sprite) sprite->SetLayer(value);
    else _layerCache.reset(value);
}

void Entity::PlayFacing(const std::string& AnimName) {
    const char* dirs[] = {"_l","_r","_u","_d"};
    std::string full = AnimName + dirs[(int)facing];
    Play(full);
}

void Entity::Play(const std::string& AnimName, bool Force, std::optional<int> newFramerate) {
    if (sprite && sprite->PlayAnim(AnimName, Force, newFramerate))
        AnimationChanged(AnimName);
}

void Entity::Draw() { DrawImpl(); }

void Entity::DrawImpl() {
    if (!exists) return;
    if (visible && sprite) {
        sprite->Draw(*Drawing::SpriteDrawer::_spriteBatch,
                     Position - offset * scale, scale, (int)y_push,
                     rotation, opacity, _flip);
    }
    if (shadow) shadow->Draw();
}

void Entity::SetFrame(int frame) {
    if (sprite) sprite->SetFrame(frame);
}

bool Entity::SetTexture(const std::string& textureName, int fw, int fh, bool ignoreChaos, bool allowFailure) {
    if (!sprite) {
        auto l = _layerCache ? _layerCache.get() : nullptr;
        sprite = std::make_unique<Base::Rendering::StaticSpriteRenderer>(textureName, fw, fh, 0, l);
        return true;
    }
    return sprite->SetTexture(textureName, fw, fh, ignoreChaos, allowFailure);
}

Facing Entity::FacingFromTouching(Touching t) {
    switch (t) {
        case Touching::LEFT:  return Facing::LEFT;
        case Touching::RIGHT: return Facing::RIGHT;
        case Touching::UP:    return Facing::UP;
        case Touching::DOWN:  return Facing::DOWN;
        default: return Facing::DOWN;
    }
}

Facing Entity::FlipFacing(Facing f) {
    switch (f) {
        case Facing::LEFT:  return Facing::RIGHT;
        case Facing::RIGHT: return Facing::LEFT;
        case Facing::UP:    return Facing::DOWN;
        default:            return Facing::UP;
    }
}

void Entity::FaceTowards(Vector2 target) {
    Vector2 dir = {target.X - Position.X, target.Y - Position.Y};
    if (std::abs(dir.X) > std::abs(dir.Y))
        facing = dir.X > 0 ? Facing::RIGHT : Facing::LEFT;
    else
        facing = dir.Y > 0 ? Facing::DOWN : Facing::UP;
}

Vector2 Entity::FacingDirection(Facing f) {
    return {
        f==Facing::RIGHT ? 1.f : (f==Facing::LEFT  ? -1.f : 0.f),
        f==Facing::DOWN  ? 1.f : (f==Facing::UP    ? -1.f : 0.f)
    };
}

void Entity::CenterOffset(bool updatePos) {
    float sw = sprite ? (float)sprite->Width()  : 16.f;
    float sh = sprite ? (float)sprite->Height() : 16.f;
    offset.X = (sw - width)  / 2.f;
    offset.Y = (sh - height) / 2.f;
    if (updatePos) {
        Position.X += offset.X;
        Position.Y += offset.Y;
    }
}

void Entity::Flicker(float duration) {
    if (duration == 0.f) { _flickering = false; visible = true; return; }
    _flickering = true;
    _flickerTimer = duration;
}

void Entity::DoFlicker() {
    _flickerFreq -= GameTimes::DeltaTime();
    if (_flickerFreq <= 0) {
        _flickerFreq = FlickerLength;
        visible = !visible;
    }
    if (_flickerTimer > 0) {
        _flickerTimer -= GameTimes::DeltaTime();
        if (_flickerTimer <= 0) { _flickering = false; visible = true; }
    }
}

void Entity::Update() {
    GameObject::Update();
    if (shadow) shadow->Update();
    if (_flickering) DoFlicker();
}

void Entity::PostUpdate() {
    GameObject::PostUpdate();
    if (sprite) sprite->Update();
    if (shadow) shadow->PostUpdate();
}

} // namespace AnodyneSharp::Entities
