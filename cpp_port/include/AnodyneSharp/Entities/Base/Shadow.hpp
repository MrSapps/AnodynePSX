#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"

namespace AnodyneSharp::Entities {

enum class ShadowType { Normal, Big, BigVertical, Tiny, RollerHorizontal, RollerVertical };

class Shadow : public Entity {
public:
    Shadow(Entity* parent, Vector2 offset, ShadowType type = ShadowType::Normal, float fps = 8.f);

    void UpdateShadow(float jumpHeight);
    void Update() override;

private:
    Entity* _parent;
    int _maxFrame = 0;
    Vector2 PositionOffset;
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::Shadow;
using AnodyneSharp::Entities::ShadowType;
