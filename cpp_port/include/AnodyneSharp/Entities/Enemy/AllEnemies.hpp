#pragma once
#include "AnodyneSharp/Entities/Base/HealthDropper.hpp"
#include "AnodyneSharp/Entities/Base/EntityPreset.hpp"

// Enemy entities stubs

namespace AnodyneSharp::Entities {

// --- Apartment Enemies ---

class Dash_Trap : public HealthDropper {
public:
    Dash_Trap(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class GasGuy : public HealthDropper {
public:
    GasGuy(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Rat : public HealthDropper {
public:
    Rat(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
private:
    Player* _pl;
};

class Silverfish : public HealthDropper {
public:
    Silverfish(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
private:
    Player* _pl;
    enum class State { IDLE, MOVING };
    State _sfState = State::IDLE;
    float _turnTimer = 0.f;
    bool SeePlayer() const;
};

class SplitBoss : public HealthDropper {
public:
    SplitBoss(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class TeleGuy : public HealthDropper {
public:
    TeleGuy(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

// --- Bedroom Enemies ---

class Annoyer : public HealthDropper {
public:
    Annoyer(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class PewLaser : public HealthDropper {
public:
    PewLaser(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Seer : public HealthDropper {
public:
    Seer(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Shieldy : public HealthDropper {
public:
    Shieldy(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Slime : public HealthDropper {
public:
    Slime(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

// --- Cell Enemies ---

class Chaser : public HealthDropper {
public:
    Chaser(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
private:
    Player* _pl;
    bool _isHorizontal;
    float _targetVel = 0.f;
};

// --- Circus Enemies ---

class CircusFolks : public HealthDropper {
public:
    CircusFolks(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Contort : public HealthDropper {
public:
    Contort(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class FirePillar : public HealthDropper {
public:
    FirePillar(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Lion : public HealthDropper {
public:
    Lion(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

// --- Crowd Enemies ---

class Dog : public HealthDropper {
public:
    Dog(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Frog : public HealthDropper {
public:
    Frog(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Person : public HealthDropper {
public:
    Person(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
private:
    float _switchTimer = 2.f;
    float _switchTimerMax = 1.3f;
    float _talkTimer = 1.f;
    void FaceRandom();
};

class Rotator : public HealthDropper {
public:
    Rotator(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class SpikeRoller : public Entity {
public:
    SpikeRoller(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class WallBoss : public HealthDropper {
public:
    WallBoss(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

// --- Etc Enemies ---

class FollowerBro : public HealthDropper {
public:
    FollowerBro(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class SageBoss : public HealthDropper {
public:
    SageBoss(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class SpaceFace : public HealthDropper {
public:
    SpaceFace(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

// --- Go Enemies ---

class BigThorn : public HealthDropper {
public:
    BigThorn(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class BlueThorn : public HealthDropper {
public:
    BlueThorn(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class BriarBossBody : public HealthDropper {
public:
    BriarBossBody(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class BriarBossFight : public HealthDropper {
public:
    BriarBossFight(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class BriarBossMain : public HealthDropper {
public:
    BriarBossMain(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class HappyThorn : public HealthDropper {
public:
    HappyThorn(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

// --- Hotel Enemies ---

class EyeBossLandPhase : public HealthDropper {
public:
    EyeBossLandPhase(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class EyeBossWaterPhase : public HealthDropper {
public:
    EyeBossWaterPhase(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Burst_Plant : public HealthDropper {
public:
    Burst_Plant(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Dustmaid : public HealthDropper {
public:
    Dustmaid(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class SteamPipe : public HealthDropper {
public:
    SteamPipe(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

// --- Redcave Enemies ---

class Four_Shooter : public HealthDropper {
public:
    Four_Shooter(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Mover : public HealthDropper {
public:
    Mover(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class OnOffLaser : public HealthDropper {
public:
    OnOffLaser(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Red_Boss : public HealthDropper {
public:
    Red_Boss(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

class Slasher : public HealthDropper {
public:
    Slasher(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
};

// --- Suburb Enemies ---

class SuburbKiller : public HealthDropper {
public:
    SuburbKiller(EntityPreset* p, Player* player);
    void Update() override;
    void Collided(Entity* other) override;
    NAMED_ENTITY_DEFAULT()
private:
    Player* _pl;
    bool _moving = false;
    float _moveTimer = 0.f;
};

} // namespace AnodyneSharp::Entities
