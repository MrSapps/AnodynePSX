// Implementations for Event entities, Lights, and Player sub-entities
#include "AnodyneSharp/Entities/EventsAndLights.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Entities/Player/Broom.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "AnodyneSharp/Utilities/MathUtilities.hpp"
#include "AnodyneSharp/GameTimes.hpp"

namespace AnodyneSharp::Entities {

// ---------------------------------------------------------------------------
// BossKeyReset — resets boss-rush key count immediately on spawn
// ---------------------------------------------------------------------------
BossKeyReset::BossKeyReset(EntityPreset*p, Player*pl)
    : Entity(Vector2{0,0}, Drawing::DrawOrder::ENTITIES)
{
    GlobalState::inventory.SetMapKeys("BOSSRUSH", 0);
    exists = false;
}

// ---------------------------------------------------------------------------
// Checkpoint — save point with stepped-on animation
// ---------------------------------------------------------------------------
Checkpoint::Checkpoint(EntityPreset*p, Player*pl)
    : Entity(p->Position,
             std::make_unique<AnimatedSpriteRenderer>("checkpoint", 16, 16,
                 std::vector<Anim>{
                     Anim("inactive",   {GlobalState::IsCell() ? 4 : 0}, 1),
                     Anim("active",     {GlobalState::IsCell() ? 5 : 1,
                                         GlobalState::IsCell() ? 6 : 2,
                                         GlobalState::IsCell() ? 7 : 3}, 10, false),
                     Anim("stepped_on", {GlobalState::IsCell() ? 5 : 1,
                                         GlobalState::IsCell() ? 6 : 2}, 12)
                 }),
             Drawing::DrawOrder::VERY_BG_ENTITIES)
    , _pl(pl), _preset(p)
{
    width = height = 8;
    offset = {4, 4};
    Position += offset;
    HasVisibleHitbox = true;

    bool isActive = (GlobalState::checkpoint.has_value() &&
                     GlobalState::checkpoint.value().map == GlobalState::CURRENT_MAP_NAME &&
                     std::abs(GlobalState::checkpoint.value().Position.X - Position.X) < 1.f &&
                     std::abs(GlobalState::checkpoint.value().Position.Y - Position.Y) < 1.f);
    Play(isActive ? "active" : "inactive");
}

bool Checkpoint::_playerOn() const {
    if (!_pl) return false;
    Rectangle pb = _pl->Hitbox();
    return pb.Intersects(Hitbox());
}

bool Checkpoint::_isActive() const {
    if (!GlobalState::checkpoint.has_value()) return false;
    return GlobalState::checkpoint->map == GlobalState::CURRENT_MAP_NAME &&
           std::abs(GlobalState::checkpoint->Position.X - Position.X) < 1.f &&
           std::abs(GlobalState::checkpoint->Position.Y - Position.Y) < 1.f;
}

void Checkpoint::Update() {
    Entity::Update();

    bool on = _playerOn();

    switch (_state) {
    case State::Wait:
        Play(_isActive() ? "active" : "inactive");
        if (on) { Play("stepped_on"); _state = State::PlayerOn; }
        break;
    case State::PlayerOn:
        if (!on) { _state = State::Wait; }
        break;
    case State::Saved:
        Play("active");
        if (!on) { _state = State::Wait; }
        break;
    }

    // Auto-save icon fade
    if (_saveIconOpacity > 0) {
        _saveIconOpacity -= GameTimes::DeltaTime() * 0.33f;
        if (_saveIconOpacity < 0) _saveIconOpacity = 0;
    }

    PostUpdate();
}

bool Checkpoint::PlayerInteraction(Facing /*dir*/) {
    if (_state == State::PlayerOn) {
        Sounds::SoundManager::PlaySoundEffect("button_down");
        if (!_isActive()) {
            GlobalState::CUR_HEALTH = GlobalState::MAX_HEALTH_get();
            GlobalState::checkpoint = GlobalState::CheckPoint{GlobalState::CURRENT_MAP_NAME, Position};
        }
        GlobalState::SaveGame();
        _saveIconOpacity = 1.f;
        _state = State::Saved;
    }
    return false;
}

// ---------------------------------------------------------------------------
// DoorToggle — collide with a Door entity and toggle its Active state
// ---------------------------------------------------------------------------
DoorToggle::DoorToggle(EntityPreset*p, Player*pl)
    : Entity(p->Position, Drawing::DrawOrder::ENTITIES) {}

DoorToggle::DoorToggle(Vector2 pos, int w, int h)
    : Entity(pos, Drawing::DrawOrder::ENTITIES) { width=w; height=h; }

void DoorToggle::Update() {
    Entity::Update();
    if (_hitDoor) exists = false;
}

// DoorToggle::Collided — keep inside namespace since _hitDoor is a private member
void DoorToggle::Collided(Entity* other) {
    // Toggle logic handled externally; just mark hit so we self-destruct
    _hitDoor = true;
}

// ---------------------------------------------------------------------------
// DungeonEntrance — records the dungeon entrance for warp-back
// ---------------------------------------------------------------------------
DungeonEntrance::DungeonEntrance(EntityPreset*p, Player*pl)
    : Entity(p->Position, Drawing::DrawOrder::ENTITIES)
{
    exists = false;
    if (GlobalState::ReturnTarget.has_value() &&
        GlobalState::ReturnTarget.value().map == GlobalState::CURRENT_MAP_NAME) return;
    GlobalState::ReturnTarget = GlobalState::CheckPoint{GlobalState::CURRENT_MAP_NAME,
                                                         p->Position - Vector2{10, 34}};
}

// ---------------------------------------------------------------------------
// FadeSwitchSong — fade out BGM, switch to new song, fade back in
// ---------------------------------------------------------------------------
FadeSwitchSong::FadeSwitchSong(EntityPreset*p, Player*pl)
    : Entity(Vector2{0,0}, Drawing::DrawOrder::ENTITIES)
{
    visible = false;
    _next = ""; // no preset TypeValue mapping here — use direct constructor instead
    exists = false;
}

FadeSwitchSong::FadeSwitchSong(const std::string& nextSong)
    : Entity(Vector2{0,0}, Drawing::DrawOrder::ENTITIES)
    , _vol(0.f, 1.6f)
    , _next(nextSong)
{
    visible = false;
}

void FadeSwitchSong::Update() {
    Entity::Update();
    _vol.Update();
    if (_vol.ReachedTarget) {
        Sounds::SoundManager::PlaySong(_next, 0);
        _vol.SetTarget(1.f);
        exists = false;
    }
}

// ---------------------------------------------------------------------------
// VolumeEvent — smoothly fades BGM volume toward a target
// ---------------------------------------------------------------------------
VolumeEvent::VolumeEvent(EntityPreset*p, Player*pl)
    : Entity(Vector2{0,0}, Drawing::DrawOrder::ENTITIES)
{
    visible = false;
    float tgt = 1.f;
    try { tgt = std::stof(p->TypeValue); } catch (...) {}
    SetTarget(tgt);
}

VolumeEvent::VolumeEvent(float target, float spd)
    : Entity(Vector2{0,0}, Drawing::DrawOrder::ENTITIES)
    , speed(spd)
{
    visible = false;
    SetTarget(target);
}

void VolumeEvent::SetTarget(float t) { _target = t; ReachedTarget = false; }

void VolumeEvent::Update() {
    Entity::Update();
    if (!ReachedTarget) {
        float cur = Sounds::SoundManager::GetVolume();
        ReachedTarget = MathUtilities::MoveTo(cur, _target, speed);
        Sounds::SoundManager::SetSongVolume(cur);
    }
}

// ---------------------------------------------------------------------------
// Lights
// ---------------------------------------------------------------------------
namespace Lights {

Light::Light(Vector2 pos, int radius)
    : Entity(pos, "light", radius*2, radius*2, Drawing::DrawOrder::DARKNESS) {
    SetSolid(false);
}
void Light::Update(){ Entity::Update(); PostUpdate(); }
void Light::Draw()  { DrawImpl(); }

PlayerLight::PlayerLight(Player* player)
    : Light(player ? player->Position : Vector2{0,0}, 32), _player(player) {}
void PlayerLight::Update(){
    if (_player) Position = _player->Position;
    Light::Update();
}

} // namespace Lights

// ---------------------------------------------------------------------------
// Player sub-entities
// ---------------------------------------------------------------------------
Foot_Overlay::Foot_Overlay(Player* player)
    : Entity(player->Position,
             std::make_unique<AnimatedSpriteRenderer>("overlay_water", 24, 24,
                 std::vector<Anim>{
                     Anim("water",     {0, 1}, 5),
                     Anim("grass_go",  {4, 5}, 8),
                     Anim("grass_stop",{5},    1)
                 }),
             Drawing::DrawOrder::FOOT_OVERLAY)
    , _player(player)
{
    visible = false;
    SetSolid(false);
}

void Foot_Overlay::OnMapChange() {
    std::string mapTex = "overlay_" + GlobalState::CURRENT_MAP_NAME + "_water";
    if (!SetTexture(mapTex, 24, 24, false, /*allowFailure=*/true)) {
        SetTexture("overlay_water", 24, 24);
        offset.Y = 0;
        set_layer(Drawing::DrawOrder::FOOT_OVERLAY);
    }
    Play("water");
}

void Foot_Overlay::_Activate() {
    if (CurAnimName() != "water") OnMapChange();
    _activated = true;
    if (GlobalState::CURRENT_MAP_NAME == "WINDMILL") {
        visible = true;
    } else {
        Flicker(0.1f);
    }
}

void Foot_Overlay::Update() {
    Entity::Update();
    HasVisibleHitbox = false;
    if (GlobalState::CURRENT_MAP_NAME == "WINDMILL") {
        visible = true;
    }
}

void Foot_Overlay::PostUpdate() {
    Entity::PostUpdate();

    Position = _player->Position - Vector2{7.f, 3.f};
    if (_player->facing == Facing::RIGHT) {
        Position.X -= 1.f;
    }

    if (!_activated || _player->state == PlayerState::AIR) {
        visible = false;
        _flickering = false;
    }
    _activated = false;
}

void Foot_Overlay::Draw() { DrawImpl(); }

void Foot_Overlay::Grass() {
    if (CurAnimName() == "water") {
        std::string grassTex = "overlay_" + GlobalState::CURRENT_MAP_NAME + "_grass";
        SetTexture(grassTex, 24, 24, false, true);
        offset.Y = 1.f;
        set_layer(Drawing::DrawOrder::FG_SPRITES);
    }
    _activated = true;
    visible = true;

    if (_player->velocity.X == 0.f && _player->velocity.Y == 0.f) {
        Play("grass_stop");
    } else {
        Play("grass_go");
    }
}

void Foot_Overlay::Puddle()              { _Activate(); }
void Foot_Overlay::Conveyor(Touching)    { _Activate(); }

// ---------------------------------------------------------------------------
PlayerReflection::PlayerReflection(Player* player)
    : Entity(player->Position, "young_player_reflection", 16, 16, Drawing::DrawOrder::PLAYER_REFLECTION)
    , _player(player)
{
    SetSolid(false);
    visible = false;
    _broomReflection = std::make_unique<Entity>(player->Position, "broom_reflection", 16, 16,
                                                Drawing::DrawOrder::PLAYER_REFLECTION);
    _broomReflection->exists = false;
}

void PlayerReflection::Update() {
    HasVisibleHitbox = false;
    Entity::Update();

    Position = _player->Position + Vector2{0.f, 7.f};
    offset = Vector2{_player->offset.X, -_player->offset.Y};
    SetFrame(_player->Frame());

    _broomReflection->visible = visible;
    _broomReflection->exists  = _player->broom ? _player->broom->exists : false;

    if (_broomReflection->exists && _player->broom) {
        _broomReflection->Position = _player->broom->Position + Vector2{0.f, 10.f};
        _broomReflection->offset   = _player->broom->offset;
        _broomReflection->SetFrame(_player->broom->Frame());

        // Mirror broom rotation
        constexpr float PI = 3.14159265f;
        switch (_player->broom->facing) {
        case Facing::RIGHT: _broomReflection->rotation = PI;          break;
        case Facing::UP:    _broomReflection->rotation = PI * 0.5f;   break;
        case Facing::DOWN:  _broomReflection->rotation = PI * 1.5f;   break;
        default:            _broomReflection->rotation = 0.f;          break;
        }
    }
}

void PlayerReflection::Reflection() { visible = true; }

void PlayerReflection::Draw() {
    Entity::Draw();
    visible = false;
}

std::vector<Entity*> PlayerReflection::SubEntities() {
    return {_broomReflection.get()};
}

} // namespace AnodyneSharp::Entities

