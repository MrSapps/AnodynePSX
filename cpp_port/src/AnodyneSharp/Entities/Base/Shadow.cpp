#include "AnodyneSharp/Entities/Base/Shadow.hpp"

namespace AnodyneSharp::Entities {

Shadow::Shadow(Entity* parent, Vector2 offset, ShadowType type, float fps)
    : Entity(parent->Position, Drawing::DrawOrder::SHADOWS) {
    _parent = parent;
    float pw = (float)(parent->sprite ? parent->sprite->Width()  : 16);
    float ph = (float)(parent->sprite ? parent->sprite->Height() : 16);
    PositionOffset = {offset.X - pw/4.f, offset.Y - ph/4.f};

    switch (type) {
        case ShadowType::Normal:
            SetTexture("8x8_shadow", 8, 8); _maxFrame = 3; break;
        case ShadowType::Big:
            SetTexture("28x10_shadow", 28, 10); _maxFrame = 4; break;
        case ShadowType::BigVertical:
            SetTexture("10x28_shadow", 10, 28); _maxFrame = 4; break;
        case ShadowType::Tiny:
            SetTexture("teeny_shadow", 3, 3); break;
        case ShadowType::RollerHorizontal:
            SetTexture("spike_roller_horizontal_shadow", 128, 16); break;
        case ShadowType::RollerVertical:
            SetTexture("spike_roller_shadow", 16, 128); break;
    }
    if (sprite) { width = sprite->Width(); height = sprite->Height(); }
}

void Shadow::UpdateShadow(float jumpHeight) {
    if (_maxFrame > 0) {
        int frame = (int)std::ceil(jumpHeight * (_maxFrame+1)) - 1;
        if (frame == -1) visible = false;
        else { visible = true; SetFrame(frame); }
    }
}

void Shadow::Update() {
    Entity::Update();
    Position = {_parent->Position.X - PositionOffset.X,
                _parent->Position.Y - PositionOffset.Y};
}

} // namespace AnodyneSharp::Entities
