#include "AnodyneSharp/Entities/Base/Rendering/SpriteRenderer.hpp"
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/GameTimes.hpp"

namespace AnodyneSharp::Entities::Base::Rendering {

// --- NonEntityLayer ---
float NonEntityLayer::Z() const {
    return Drawing::DrawingUtilities::GetDrawingZ(layer);
}

// --- Layer ---
float Layer::Z() const {
    auto* e = static_cast<AnodyneSharp::Entities::Entity*>(parent);
    float y = e ? e->Position.Y : 0.f;
    using namespace Drawing;
    switch (layer) {
        case DrawOrder::ENTITIES:
        case DrawOrder::FG_SPRITES:
        case DrawOrder::BG_ENTITIES:
        case DrawOrder::PARTICLES:
            return DrawingUtilities::GetDrawingZ(layer, y);
        default:
            return DrawingUtilities::GetDrawingZ(layer);
    }
}

// --- AnimatedSpriteRenderer constructors ---
AnimatedSpriteRenderer::AnimatedSpriteRenderer(
        const std::string& textureName, int frameWidth, int frameHeight,
        std::vector<Anim> animations)
    : _textureName(textureName), _layer(nullptr)
{
    auto* handle = Resources::ResourceManager::GetTexHandle(textureName, false, true);
    _sprite = Spritesheet(handle, frameWidth, frameHeight);
    for (auto& a : animations) _animations[a.name] = std::move(a);
    if (!_animations.empty()) _curAnim = &_animations.begin()->second;
}

AnimatedSpriteRenderer::AnimatedSpriteRenderer(
        const std::string& textureName, int frameWidth, int frameHeight,
        ILayerType* layer, std::vector<Anim> animations)
    : _textureName(textureName), _layer(layer)
{
    auto* handle = Resources::ResourceManager::GetTexHandle(textureName, false, true);
    _sprite = Spritesheet(handle, frameWidth, frameHeight);
    for (auto& a : animations) _animations[a.name] = std::move(a);
    if (!_animations.empty()) _curAnim = &_animations.begin()->second;
}

const std::string& AnimatedSpriteRenderer::CurAnimName() const {
    static const std::string empty;
    return _curAnim ? _curAnim->name : empty;
}

void AnimatedSpriteRenderer::Draw(SpriteBatch& /*batch*/, const Vector2& position,
                                   float scale, int y_push, float rotation,
                                   float opacity, SpriteEffects flip) {
    auto* tex = _sprite.texHandle ? _sprite.texHandle->Tex() : nullptr;
    if (!tex) return;
    int frame = _curAnim ? _curAnim->Frame() : 0;
    auto src = _sprite.GetRect(frame);
    float z = _layer ? _layer->Z() : 0.f;
    Color tint = _color;
    tint.A = static_cast<uint8_t>(opacity * 255.f);
    Drawing::SpriteDrawer::DrawSprite(
        tex, Vector2{position.X, position.Y + (float)y_push},
        &src, &tint, rotation, scale, z);
}

void AnimatedSpriteRenderer::SetFrame(int /*index*/) {}

bool AnimatedSpriteRenderer::PlayAnim(const std::string& name, bool force,
                                       std::optional<int> newFramerate) {
    auto it = _animations.find(name);
    if (it == _animations.end()) return false;
    if (!force && _curAnim == &it->second) return false;
    _curAnim = &it->second;
    _curAnim->Reset();
    if (newFramerate) _curAnim->SetFrameRate(static_cast<float>(*newFramerate));
    return true;
}

bool AnimatedSpriteRenderer::SetTexture(const std::string& textureName,
                                         int width, int height,
                                         bool ignoreChaos, bool allowFailure) {
    auto* handle = Resources::ResourceManager::GetTexHandle(textureName, ignoreChaos, allowFailure);
    if (!handle && !allowFailure) return false;
    _sprite = Spritesheet(handle, width, height);
    _textureName = textureName;
    return true;
}

void AnimatedSpriteRenderer::Update() {
    if (_curAnim) _curAnim->Update();
}

// --- StaticSpriteRenderer ---
StaticSpriteRenderer::StaticSpriteRenderer(
        const std::string& textureName, int frameWidth, int frameHeight,
        int frame, ILayerType* layer, bool ignoreChaos)
    : _textureName(textureName), _curFrame(frame), _layer(layer)
{
    auto* handle = Resources::ResourceManager::GetTexHandle(textureName, ignoreChaos, true);
    _sprite = Spritesheet(handle, frameWidth, frameHeight);
}

void StaticSpriteRenderer::Draw(SpriteBatch& /*batch*/, const Vector2& position,
                                  float scale, int y_push, float rotation,
                                  float opacity, SpriteEffects flip) {
    auto* tex = _sprite.texHandle ? _sprite.texHandle->Tex() : nullptr;
    if (!tex) return;
    auto src = _sprite.GetRect(_curFrame);
    float z = _layer ? _layer->Z() : 0.f;
    Color tint = _color;
    tint.A = static_cast<uint8_t>(opacity * 255.f);
    Drawing::SpriteDrawer::DrawSprite(
        tex, Vector2{position.X, position.Y + (float)y_push},
        &src, &tint, rotation, scale, z);
}

bool StaticSpriteRenderer::SetTexture(const std::string& textureName,
                                        int width, int height,
                                        bool ignoreChaos, bool allowFailure) {
    auto* handle = Resources::ResourceManager::GetTexHandle(textureName, ignoreChaos, allowFailure);
    if (!handle && !allowFailure) return false;
    _sprite = Spritesheet(handle, width, height);
    _textureName = textureName;
    return true;
}

// --- SolidColorRenderer ---
void SolidColorRenderer::Draw(SpriteBatch& /*batch*/, const Vector2& /*position*/,
                               float /*scale*/, int /*y_push*/, float /*rotation*/,
                               float /*opacity*/, SpriteEffects /*flip*/) {}

} // namespace AnodyneSharp::Entities::Base::Rendering
