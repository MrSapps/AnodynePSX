#pragma once
#include <string>
#include <memory>
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/EntityPreset.hpp"

// Gadget entities - interactive environmental objects

namespace AnodyneSharp::Entities {

class Player; // forward
struct DoorMapPair; // forward

// BaseTreasure declared before TreasureChest so TreasureChest can own one
class BaseTreasure : public Entity {
public:
    int _dialogueID = -1;
    BaseTreasure(Vector2 pos, Drawing::DrawOrder layer) : Entity(pos, layer) {}
    virtual ~BaseTreasure() = default;
    virtual void GetTreasure();
};


// Blockers
class Blocker : public Entity {
public:
    Blocker(EntityPreset* p, Player* pl);
    void Update() override;
    NAMED_ENTITY_DEFAULT()
private: Player* _pl;
};

// Button
class Button : public Entity {
public:
    Button(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
private:
    bool _pressed=false, _incremented=false, _permanent=false;
};

// Console
class Console : public Entity {
public:
    Console(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    bool PlayerInteraction(Facing d);
    NAMED_ENTITY_DEFAULT()
private: EntityPreset* _preset;
};

// DashPad
class DashPad : public Entity {
public:
    DashPad(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
private: float _t = 0.f;
};

// Door
class Door : public Entity {
public:
    bool Active = true;
    Door(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
protected:
    Player*      _pl;
    bool         _onDoor = false;
    Vector2      teleportOffset{0.f, 0.f};
    std::string  _sfx{"enter_door"};
    DoorMapPair* _linked = nullptr;
    virtual void TeleportPlayer();
};

// Door variants
class BlankPortal     : public Door { public: BlankPortal(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT() };
class DirectionalDoor : public Door {
public:
    DirectionalDoor(EntityPreset*p,Player*pl);
    void Collided(Entity*o) override;
    NAMED_ENTITY_DEFAULT()
protected:
    Facing _exitDir{Facing::DOWN};
    void TeleportPlayer() override;
};
class FallDoor : public Door {
public:
    FallDoor(EntityPreset*p,Player*pl);
    void Collided(Entity*o) override;
    NAMED_ENTITY_DEFAULT()
private: bool _smack=true;
};
class NexusDoor : public Door { public: NexusDoor(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT() };
class NexusPad  : public Door {
public:
    NexusPad(EntityPreset*p,Player*pl);
    void Update() override;
    void Collided(Entity*o) override;
    bool PlayerInteraction(Facing d);
    NAMED_ENTITY_DEFAULT()
private: bool _onPad=false;
};
class NoMoveDoor : public Door { public: NoMoveDoor(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT() };
class OneWayDoor : public Door { public: OneWayDoor(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT() };
class WhirlPool  : public Door {
public:
    WhirlPool(EntityPreset*p,Player*pl);
    void Update() override;
    void DoTransition();
    NAMED_ENTITY_DEFAULT()
private: EntityPreset* _preset;
};

// Dust
class Dust : public Entity {
public:
    Dust(Vector2 pos, bool isRaft=false);
    void Update() override;
    void Collided(Entity* o) override;
    bool IsRaft=false, ON_CONVEYOR=false;
    NAMED_ENTITY_DEFAULT()
};

// Explosion
class Explosion : public Entity {
public:
    Explosion(Vector2 pos);
    void Update() override;
};

// Gate
class Gate : public Entity {
public:
    Gate(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
private:
    EntityPreset* _preset;
    Player*       _pl;
    bool          _heldDown=false;
    bool ConditionSatisfied() const;
};
class BigCardGate  : public Gate { public: BigCardGate(EntityPreset*p,Player*pl);  NAMED_ENTITY_DEFAULT() };
class BigGate      : public Gate { public: BigGate(EntityPreset*p,Player*pl);      NAMED_ENTITY_DEFAULT() };
class BigKeyGate   : public Gate { public: BigKeyGate(EntityPreset*p,Player*pl);   NAMED_ENTITY_DEFAULT() };
class KeyBlockSentinel : public Entity { public: KeyBlockSentinel(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT() };
class KeyCardGate  : public Gate { public: KeyCardGate(EntityPreset*p,Player*pl);  NAMED_ENTITY_DEFAULT() };
class SmallKeyGate : public Gate { public: SmallKeyGate(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT() };

// Detectors
class GoDetector : public Entity {
public:
    GoDetector(EntityPreset*p,Player*pl);
    void Update() override;
    NAMED_ENTITY_DEFAULT()
private:
    EntityPreset* _preset;
};
class GoHappyBlocker : public Entity {
public:
    GoHappyBlocker(EntityPreset*p,Player*pl);
    void Collided(Entity*o) override;
    NAMED_ENTITY_DEFAULT()
};
class GoQuestDoorBlocker : public Entity { public: GoQuestDoorBlocker(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT() };

// HealthEntity
class HealthEntity : public Entity {
public:
    HealthEntity(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
private:
    EntityPreset* _preset;
    bool _isLarge;
};

// Holes
class Hole : public Entity {
public:
    Hole(EntityPreset*p,Player*pl);
    void Collided(Entity*o) override;
    NAMED_ENTITY_DEFAULT()
};
class CrackedTile : public Entity {
public:
    CrackedTile(EntityPreset*p,Player*pl);
    void Collided(Entity*o) override;
    NAMED_ENTITY_DEFAULT()
private: float _t=1.f;
};

// JumpTrigger
class JumpTrigger : public Entity { public: JumpTrigger(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT() };

// Key
class Key : public Entity {
public:
    Key(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
private:
    EntityPreset* _preset;
    bool _bossRush=false;
};

// Propelled
class Propelled : public Entity { public: Propelled(Vector2 pos,Vector2 vel,float lifetime); void Update() override; };

// SoundTestConsole
class SoundTestConsole : public Entity {
public:
    SoundTestConsole(EntityPreset*p,Player*pl);
    bool PlayerInteraction(Facing d);
    NAMED_ENTITY_DEFAULT()
};

// SpringPad
class SpringPad : public Entity {
public:
    SpringPad(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
private:
    int   _dist=32;
    float _jumpT=0.3f;
    bool  _playerCol=false, _activated=false;
};

// Switches
class PillarSwitch : public Entity {
public:
    PillarSwitch(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
private: float _hitTm=0.f;
};
class SwitchPillar : public Entity {
public:
    SwitchPillar(EntityPreset* p, Player* pl);
    void Update() override;
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
private:
    int _defFrame=0, _cur=0;
    static constexpr int UP_FRAME=0, DOWN_FRAME=1;
    int TargetFrame() const;
};

// TreasureChest
class TreasureChest : public Entity {
public:
    bool opened=false;
    TreasureChest(EntityPreset* p, Player* pl);
    void Update() override;
    bool PlayerInteraction(Facing d);
    void Collided(Entity* o) override;
    NAMED_ENTITY_DEFAULT()
private:
    EntityPreset*                 _preset;
    std::unique_ptr<BaseTreasure> _treasure;
    void SetTreasure();
};

// Treasure flyup
class Treasure : public BaseTreasure {
public:
    Treasure(Vector2 pos);
    void Update() override;
    void GetTreasure() override;
private: Vector2 _end;
};

// Specific treasures
class BootsTreasure  : public BaseTreasure { public: BootsTreasure(EntityPreset*p,Player*pl);  void GetTreasure() override; NAMED_ENTITY_DEFAULT() };
class BroomTreasure  : public BaseTreasure { public: BroomTreasure(EntityPreset*p,Player*pl);  void GetTreasure() override; NAMED_ENTITY_DEFAULT() };
class CardTreasure   : public BaseTreasure { public: CardTreasure(EntityPreset*p,Player*pl);   void GetTreasure() override; NAMED_ENTITY_DEFAULT() };
class KeyTreasure    : public BaseTreasure { public: KeyTreasure(EntityPreset*p,Player*pl);    void GetTreasure() override; NAMED_ENTITY_DEFAULT() };
class SecretTreasure : public BaseTreasure { public: SecretTreasure(EntityPreset*p,Player*pl); void GetTreasure() override; NAMED_ENTITY_DEFAULT() };

// WaterAnim
class WaterAnim : public Entity { public: WaterAnim(EntityPreset*p,Player*pl); void Update() override; NAMED_ENTITY_DEFAULT() };

} // namespace AnodyneSharp::Entities
