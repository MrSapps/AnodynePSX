#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/EntityPreset.hpp"

namespace AnodyneSharp::Entities::Interactive { class HealthPickup; }

namespace AnodyneSharp::Entities {

class HealthDropper : public Entity {
public:
    HealthDropper(EntityPreset* p, Vector2 pos, std::unique_ptr<ISpriteRenderer> sprite,
                  Drawing::DrawOrder layer, float healthDropChance = 0.5f, bool dropBigHealth = false);
    ~HealthDropper();

    std::vector<Entity*> SubEntities() override;

protected:
    virtual void Die();

private:
    float _healthDropChance = 0.5f;
    std::unique_ptr<class Interactive::HealthPickup> _health;
    EntityPreset* _preset;
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::HealthDropper;
