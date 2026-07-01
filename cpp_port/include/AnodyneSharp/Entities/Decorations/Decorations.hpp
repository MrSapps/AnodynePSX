#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/EntityPreset.hpp"
#include "AnodyneSharp/Entities/Interactive/AllInteractive.hpp"

// Decorations - non-interactive environment entities

namespace AnodyneSharp::Entities {

// BigTree - large solid tree
class BigTree : public Entity {
public:
    BigTree(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

// DeathFadeIn - fadeout overlay used when player dies
class DeathFadeIn : public Entity {
public:
    DeathFadeIn(Color color = Color::White);
    void Update() override;
    void Draw()   override;
    bool IsDone() const { return _done; }
private:
    bool  _done  = false;
    Color _color = Color::White;
};

// Eye_Light - ambient light effect in eye boss area
class Eye_Light : public Entity {
public:
    Eye_Light(EntityPreset* p, Player* player);
    void Update() override;
    NAMED_ENTITY_DEFAULT()
};

// NonSolid - collision-free decoration entity
class NonSolid : public Entity {
public:
    NonSolid(EntityPreset* p, Player* player);
    void Update() override;
    NAMED_ENTITY_DEFAULT()
};

// RedCaveEntrance - red cave gate (disappears if enough events)
class RedCaveEntrance : public Entity {
public:
    RedCaveEntrance(EntityPreset* p, int required_events);
    void Update() override;
    void Collided(Entity* other) override;
};
class RedCaveLeft   : public RedCaveEntrance {
public:
    RedCaveLeft(EntityPreset* p, Player* player) : RedCaveEntrance(p, 1) {}
    NAMED_ENTITY_DEFAULT()
};
class RedCaveRight  : public RedCaveEntrance {
public:
    RedCaveRight(EntityPreset* p, Player* player) : RedCaveEntrance(p, 1) {}
    NAMED_ENTITY_DEFAULT()
};
class RedCaveCenter : public RedCaveEntrance {
public:
    RedCaveCenter(EntityPreset* p, Player* player) : RedCaveEntrance(p, 0) {}
    NAMED_ENTITY_DEFAULT()
};
class RedCaveNorth  : public RedCaveEntrance {
public:
    RedCaveNorth(EntityPreset* p, Player* player) : RedCaveEntrance(p, 2) {}
    NAMED_ENTITY_DEFAULT()
};

// Sign - readable sign decoration
class Sign : public Entity, public Interactable {
public:
    Sign(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    bool PlayerInteraction(Facing dir) override;
    NAMED_ENTITY_DEFAULT()
};

// PlayerDieDummy - dummy entity for death animation
class PlayerDieDummy : public Entity {
public:
    PlayerDieDummy(Vector2 pos);
    void Update() override;
    void Draw()   override;
};

} // namespace AnodyneSharp::Entities
