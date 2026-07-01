#include "AnodyneSharp/Entities/Base/HealthDropper.hpp"
#include "AnodyneSharp/Entities/Interactive/AllInteractive.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"

namespace AnodyneSharp::Entities {

HealthDropper::~HealthDropper() = default;

HealthDropper::HealthDropper(EntityPreset* p, Vector2 pos,
    std::unique_ptr<ISpriteRenderer> spr,
    Drawing::DrawOrder layer, float healthDropChance, bool dropBigHealth)
    : Entity(pos, std::move(spr), layer),
      _healthDropChance(healthDropChance),
      _preset(p),
      _health(std::make_unique<Interactive::HealthPickup>(pos, dropBigHealth)) {}

std::vector<Entity*> HealthDropper::SubEntities() {
    return { _health.get() };
}

void HealthDropper::Die() {
    if (_preset) _preset->SetAlive(false);
    exists = false;
    if (GlobalState::settings.guaranteed_health ||
        GlobalState::RNG.NextDouble() < _healthDropChance) {
        _health->exists   = true;
        _health->Position = Position;
    }
}

} // namespace AnodyneSharp::Entities
