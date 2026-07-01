#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/EntityPreset.hpp"

// All Interactive entities (NPCs, pickups, etc.)

namespace AnodyneSharp::Entities {

class Player; // forward

// HealthPickup
namespace Interactive {
class HealthPickup : public Entity {
public:
    HealthPickup(Vector2 pos, bool big = false);
    void Update() override;
    void Collided(Entity* other) override;
private:
    int   _healFactor = 1;
    float _latency    = 0.5f;
};
}

// Interactable interface
class Interactable {
public:
    virtual ~Interactable() = default;
    virtual bool PlayerInteraction(Facing player_direction) = 0;
};

// DungeonStatue
class DungeonStatue : public Entity, public Interactable{
public: DungeonStatue(EntityPreset*p,Player*pl); bool PlayerInteraction(Facing d)override; NAMED_ENTITY_DEFAULT()
private: EntityPreset* _preset;};

// Elevator
class Elevator : public Entity {
public: Elevator(EntityPreset*p,Player*pl); void Update()override; NAMED_ENTITY_DEFAULT()};

// HealthCicada
class HealthCicada : public Entity {
public:
    HealthCicada(EntityPreset*p,Player*pl);
    void Update() override;
    void Collided(Entity*o) override;
    NAMED_ENTITY_DEFAULT()
private:
    enum class State { WAIT_BOSS, FLYING, ACTIVE, DONE };
    State       _state = State::WAIT_BOSS;
    EntityPreset* _preset;
    Player*     _pl;
    Vector2     _targetPos{0,0};
    float       _t = 0.f;
};

class HealthCicadaSentinel : public Entity {
public: HealthCicadaSentinel(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT()};

// Red_Pillar
class Red_Pillar : public Entity {
public: Red_Pillar(EntityPreset*p,Player*pl); void Update()override; NAMED_ENTITY_DEFAULT()};

// NPC stubs (many NPCs follow same pattern)
#define NPC_STUB(ClassName) \
class ClassName : public Entity, public Interactable { \
public: ClassName(EntityPreset*p,Player*pl); \
    bool PlayerInteraction(Facing d) override; \
    void Update() override; \
    NAMED_ENTITY_DEFAULT() \
};

NPC_STUB(BeachBriar)
NPC_STUB(Fisherman)
NPC_STUB(BombDude)
NPC_STUB(HairDude)
NPC_STUB(RedWalker)
NPC_STUB(BlueBriar)
NPC_STUB(Dam)
NPC_STUB(Snowman)
NPC_STUB(CellBody)
NPC_STUB(ArthurDanger)
NPC_STUB(CircusFolksDead)
NPC_STUB(JavieraDanger)
NPC_STUB(CliffDog)
NPC_STUB(BioFilm)
NPC_STUB(DeathPlace)
NPC_STUB(DevEaster)
NPC_STUB(ApartmentEaster)
NPC_STUB(RedCaveEaster)
NPC_STUB(FieldsEaster)
NPC_STUB(Olive)
NPC_STUB(Bunny)
NPC_STUB(ForestBriar)
NPC_STUB(HugeFuckingStag)
NPC_STUB(Mushroom)
NPC_STUB(SkittishSecret)
NPC_STUB(Thorax)
NPC_STUB(EndingSage)
NPC_STUB(PostBlue)
NPC_STUB(HappyBriar)
NPC_STUB(HappyEventTrigger)
NPC_STUB(HappyNPC)
NPC_STUB(EyebossPreview)
NPC_STUB(HotelGuy)
NPC_STUB(Mitra)
NPC_STUB(BeachQuest)
NPC_STUB(CellQuest)
NPC_STUB(CliffQuest)
NPC_STUB(FieldsQuest)
NPC_STUB(ForestQuest)
NPC_STUB(GoQuest)
NPC_STUB(SpaceQuest)
NPC_STUB(SuburbQuest)
NPC_STUB(BlankConsole)
NPC_STUB(Icky)
NPC_STUB(InsideMonster)
NPC_STUB(MiaoXiao)
NPC_STUB(OutsideMonster)
NPC_STUB(ShopKeep)
NPC_STUB(Sage)
NPC_STUB(ShadowBriar)
NPC_STUB(CubeKing)
NPC_STUB(SpaceNPC)
NPC_STUB(Statue)
NPC_STUB(Sadbro)
NPC_STUB(SuburbBlocker)
NPC_STUB(SuburbIndoors)
NPC_STUB(SuburbWalker)
NPC_STUB(WindmillConsole)
NPC_STUB(WindmillShell)

// Rock and Big_Key have real implementations
class Rock : public Entity, public Interactable {
public:
    Rock(EntityPreset*p,Player*pl);
    void Collided(Entity*o) override;
    bool PlayerInteraction(Facing d) override;
    void Update() override;
    NAMED_ENTITY_DEFAULT()
private:
    std::string _scene;
};

class Big_Key : public Entity, public Interactable {
public:
    Big_Key(EntityPreset*p,Player*pl);
    void Update() override;
    bool PlayerInteraction(Facing d) override;
    NAMED_ENTITY_DEFAULT()
private:
    enum class State { IDLE, ANIMATING, DONE };
    State         _state = State::IDLE;
    EntityPreset* _preset;
    Player*       _pl;
    float         _orbitRadius = 0.f;
    float         _timer = 0.f;
    float         _orbitAngle = 0.f;
};

// Debug NPCs
NPC_STUB(Big_Door)
NPC_STUB(Black_Thing)
NPC_STUB(TestNPC)

#undef NPC_STUB

} // namespace AnodyneSharp::Entities
