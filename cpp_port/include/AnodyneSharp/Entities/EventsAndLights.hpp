#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/EntityPreset.hpp"

// Event entities, Lights, Player sub-entities, DoorPair support

namespace AnodyneSharp::Entities {

class Player; // forward

// --- Events ---
class BossKeyReset : public Entity {
public: BossKeyReset(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT()};

class Checkpoint : public Entity {
public:
    Checkpoint(EntityPreset*p,Player*pl);
    void Update() override;
    bool PlayerInteraction(Facing dir);
    NAMED_ENTITY_DEFAULT()
private:
    enum class State { Wait, PlayerOn, Saved };
    State _state = State::Wait;
    float _saveIconOpacity = 0.f;
    EntityPreset* _preset = nullptr;
    Player* _pl = nullptr;
    bool _playerOn() const;
    bool _isActive() const;
};

class DoorToggle : public Entity {
public:
    DoorToggle(EntityPreset*p,Player*pl);
    DoorToggle(Vector2 pos, int w, int h);
    void Update()  override;
    void Collided(Entity* other) override;
    bool Active = true;
    NAMED_ENTITY_DEFAULT()
private:
    bool _hitDoor = false;
};

class DungeonEntrance : public Entity {
public: DungeonEntrance(EntityPreset*p,Player*pl); NAMED_ENTITY_DEFAULT()};

// VolumeEvent declared before FadeSwitchSong so FadeSwitchSong can hold one by value
class VolumeEvent : public Entity {
public:
    VolumeEvent(EntityPreset*p,Player*pl);
    VolumeEvent(float target, float speed = 0.4f);
    void Update() override;
    void SetTarget(float t);
    bool ReachedTarget = false;
    float speed = 0.4f;
    NAMED_ENTITY_DEFAULT()
private:
    float _target = 1.f;
};

class FadeSwitchSong : public Entity {
public:
    FadeSwitchSong(EntityPreset*p,Player*pl);
    FadeSwitchSong(const std::string& nextSong);
    void Update() override;
private:
    std::string _next;
    VolumeEvent _vol{0.f, 1.6f};
};

// --- Lights ---
namespace Lights {

class Light : public Entity {
public:
    Light(Vector2 pos, int radius = 32);
    void Update() override;
    void Draw()   override;
};

class PlayerLight : public Light {
public:
    PlayerLight(Player* player);
    void Update() override;
private:
    Player* _player;
};

} // namespace Lights

// --- Player sub-entities ---
class Foot_Overlay : public Entity {
public:
    Foot_Overlay(Player* player);
    void Update()    override;
    void PostUpdate() override;
    void Draw()      override;
    void OnMapChange();
    void Grass()     override;
    void Puddle()    override;
    void Conveyor(Touching direction) override;
    void Fall(Vector2) override {}
private:
    Player* _player;
    bool    _activated = true;
    void    _Activate();
};

class PlayerReflection : public Entity {
public:
    PlayerReflection(Player* player);
    void Update()     override;
    void Draw()       override;
    void Reflection() override;
    void Fall(Vector2) override {}
    std::vector<Entity*> SubEntities() override;
private:
    Player* _player;
    std::unique_ptr<Entity> _broomReflection;
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::Lights::Light;
using AnodyneSharp::Entities::Lights::PlayerLight;
