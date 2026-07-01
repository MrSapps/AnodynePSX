// Enemy entity implementations
#include "AnodyneSharp/Entities/Enemy/AllEnemies.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Entities/Player/Broom.hpp"
#include "AnodyneSharp/Entities/Gadget/AllGadgets.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Utilities/MathUtilities.hpp"
#include <cmath>

namespace AnodyneSharp::Entities {

// Helper macro: implement Collided for simple enemies (1 damage to player, die to broom)
#define ENEMY_COLLIDE(ClassName, damage)                             \
void ClassName::Collided(Entity* o) {                                \
    if (auto* p = dynamic_cast<Player*>(o)) {                        \
        p->ReceiveDamage(damage, #ClassName);                        \
    } else if (dynamic_cast<Broom*>(o)) {                            \
        GlobalState::SpawnEntity(new Explosion(Position));           \
        Die();                                                       \
    }                                                                \
}

// ===========================================================================
// Apartment
// ===========================================================================
Dash_Trap::Dash_Trap(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("dash_trap", 16, 16,
            Anim("idle", {0,1}, 8)), Drawing::DrawOrder::ENTITIES) {}
void Dash_Trap::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Dash_Trap, 1)

GasGuy::GasGuy(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("gas_guy", 16, 24,
            Anim("walk", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void GasGuy::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(GasGuy, 2)

// ---------------------------------------------------------------------------
// Rat — bouncer using velocity rotation on wall contact
// ---------------------------------------------------------------------------
Rat::Rat(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("rat", 16, 16,
            Anim("walk_d", {0,1}, 5),
            Anim("walk_l", {2,3}, 5),
            Anim("walk_r", {2,3}, 5),
            Anim("walk_u", {4,5}, 5)), Drawing::DrawOrder::ENTITIES)
    , _pl(pl)
{
    width = height = 12;
    CenterOffset();
    Position += offset;
    facing = Facing::DOWN;
    velocity = {0.f, 40.f};
    Play("walk_d");
}
void Rat::Update() {
    Entity::Update();
    if (touching != Touching::NONE) {
        touching = Touching::NONE;
        // Rotate velocity 90 degrees on wall hit
        float tmp = velocity.X;
        velocity.X = -velocity.Y;
        velocity.Y = tmp;
        FaceTowards(Position + velocity);
        PlayFacing("walk");
        Sounds::SoundManager::PlaySoundEffect("rat_move");
    }
    PostUpdate();
}
void Rat::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o)) {
        p->ReceiveDamage(1, "Rat");
    } else if (dynamic_cast<Broom*>(o)) {
        GlobalState::SpawnEntity(new Explosion(Position));
        Die();
    }
}

// ---------------------------------------------------------------------------
// Silverfish — idle until aligned with player, then rush; turn 90° if stuck
// ---------------------------------------------------------------------------
Silverfish::Silverfish(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("silverfish", 16, 16,
            Anim("move_l", {6,7}, 7),
            Anim("move_d", {4,5}, 7),
            Anim("move_r", {6,7}, 7),
            Anim("move_u", {8,9}, 7),
            Anim("idle_r", {6}, 12),
            Anim("idle_d", {4}, 12),
            Anim("idle_u", {8}, 12),
            Anim("idle_l", {6}, 12)), Drawing::DrawOrder::ENTITIES)
    , _pl(pl)
{
    facing = [&]{
        switch (p->Frame_) {
        case 0: return Facing::LEFT;
        case 1: return Facing::DOWN;
        case 2: return Facing::RIGHT;
        default: return Facing::UP;
        }
    }();
    PlayFacing("idle");
}
bool Silverfish::SeePlayer() const {
    auto hb = Hitbox();
    auto ph = _pl->Hitbox();
    bool sameRow = ph.Top() <= hb.Bottom() && ph.Bottom() >= hb.Top();
    bool sameCol = ph.Left() <= hb.Right() && ph.Right() >= hb.Left();
    float dist = (Center() - _pl->Center()).Length();
    return sameRow || sameCol || dist < 30.f;
}
void Silverfish::Update() {
    Entity::Update();
    if (_sfState == State::IDLE) {
        _turnTimer += GameTimes::DeltaTime();
        if (_turnTimer > 0.8f) {
            _turnTimer = 0.f;
            // Rotate facing 90°: LEFT→UP→RIGHT→DOWN→LEFT
            switch (facing) {
            case Facing::LEFT:  facing = Facing::UP;    break;
            case Facing::UP:    facing = Facing::RIGHT; break;
            case Facing::RIGHT: facing = Facing::DOWN;  break;
            default:            facing = Facing::LEFT;  break;
            }
            PlayFacing("idle");
        }
        if (SeePlayer()) {
            Sounds::SoundManager::PlaySoundEffect("sf_move");
            facing = FlipFacing(facing);
            PlayFacing("move");
            velocity = FacingDirection(facing) * 50.f;
            _sfState = State::MOVING;
        }
    } else {
        if (touching != Touching::NONE) {
            touching = Touching::NONE;
            velocity = {0.f, 0.f};
            PlayFacing("idle");
            _turnTimer = 0.f;
            _sfState = State::IDLE;
        }
    }
    PostUpdate();
}
void Silverfish::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o)) {
        p->ReceiveDamage(1, "Silverfish");
    } else if (dynamic_cast<Broom*>(o)) {
        GlobalState::SpawnEntity(new Explosion(Position));
        Die();
    }
}

SplitBoss::SplitBoss(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("splitboss", 24, 32,
            Anim("idle", {0,1}, 5)), Drawing::DrawOrder::ENTITIES) {}
void SplitBoss::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(SplitBoss, 2)

TeleGuy::TeleGuy(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("teleguy", 16, 16,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void TeleGuy::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(TeleGuy, 1)

// ===========================================================================
// Bedroom
// ===========================================================================
Annoyer::Annoyer(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("annoyer", 16, 16,
            Anim("flap", {0,1,2,3,4,5}, 8)), Drawing::DrawOrder::ENTITIES) {}
void Annoyer::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Annoyer, 1)

PewLaser::PewLaser(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("pew_laser", 16, 16,
            Anim("idle", {0}, 1)), Drawing::DrawOrder::ENTITIES) {}
void PewLaser::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(PewLaser, 1)

Seer::Seer(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("seer", 16, 16,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void Seer::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Seer, 1)

Shieldy::Shieldy(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("shieldy", 16, 16,
            Anim("idle", {0,1}, 5)), Drawing::DrawOrder::ENTITIES) {}
void Shieldy::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Shieldy, 1)

Slime::Slime(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("slime", 16, 16,
            Anim("Move", {0,1}, 3)), Drawing::DrawOrder::ENTITIES) {}
void Slime::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Slime, 1)

// ===========================================================================
// Cell
// ===========================================================================

// ---------------------------------------------------------------------------
// Chaser — axis-locked bouncer; speeds up on BroomUsed event, deals 6 damage
// ---------------------------------------------------------------------------
Chaser::Chaser(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("chaser", 16, 32,
            Anim("face_d",    {4},     1),
            Anim("face_r",    {6},     1),
            Anim("walking_d", {4,5},   8),
            Anim("walking_r", {6,7},   8),
            Anim("walking_u", {8,9},   8),
            Anim("walking_l", {10,11}, 8)), Drawing::DrawOrder::ENTITIES)
    , _pl(pl)
    , _isHorizontal(p->Frame_ == 0)
{
    width = height = 8;
    offset = {4.f, 20.f};
    Position += offset;
    if (_isHorizontal) Play("face_r"); else Play("face_d");
}
void Chaser::Update() {
    Entity::Update();
    if (velocity.X == 0.f && velocity.Y == 0.f) {
        // Check if player is aligned
        if (_isHorizontal) {
            if (_pl->Position.Y > Position.Y - (float)_pl->height &&
                _pl->Position.Y < Position.Y + (float)height) {
                FaceTowards(_pl->Position);
                velocity = FacingDirection(facing) * 15.f;
                if (velocity.X != 0.f) PlayFacing("walking");
            }
        } else {
            if (_pl->Position.X > Position.X - (float)_pl->width &&
                _pl->Position.X < Position.X + (float)width) {
                FaceTowards(_pl->Position);
                velocity = FacingDirection(facing) * 15.f;
                if (velocity.Y != 0.f) PlayFacing("walking");
            }
        }
    } else {
        if (touching != Touching::NONE) {
            velocity *= -1.f;
            facing = FlipFacing(facing);
            PlayFacing("walking");
        }
        if (_targetVel > 0.f) {
            // Speed up
            float speed = velocity.Length();
            speed = std::min(speed * 1.6f, 100.f);
            speed = std::min(speed, _targetVel);
            if (velocity.X != 0.f) velocity.X = (velocity.X > 0.f ? 1.f : -1.f) * speed;
            else                    velocity.Y = (velocity.Y > 0.f ? 1.f : -1.f) * speed;
            _targetVel = 0.f;
        }
    }
    PostUpdate();
}
void Chaser::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o)) {
        p->ReceiveDamage(6, "Chaser");
    }
    // Chaser does NOT die to broom — broom event triggers speed-up
}

// ===========================================================================
// Circus
// ===========================================================================
CircusFolks::CircusFolks(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("circus_folk", 16, 16,
            Anim("walk", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void CircusFolks::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(CircusFolks, 1)

Contort::Contort(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("contort_small", 16, 16,
            Anim("idle", {0,1}, 9)), Drawing::DrawOrder::ENTITIES) {}
void Contort::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Contort, 1)

FirePillar::FirePillar(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("fire_pillar", 16, 32,
            Anim("idle", {0}, 9)), Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void FirePillar::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(FirePillar, 1)

Lion::Lion(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("lion", 32, 32,
            Anim("idle", {10,11}, 4)), Drawing::DrawOrder::ENTITIES) {}
void Lion::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Lion, 1)

// ===========================================================================
// Crowd
// ===========================================================================
Dog::Dog(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("dog", 16, 16,
            Anim("stop",   {0},    1),
            Anim("walk",   {2,3},  4),
            Anim("alert",  {4,5},  4),
            Anim("attack", {6,7},  6)), Drawing::DrawOrder::ENTITIES) {}
void Dog::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Dog, 1)

Frog::Frog(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("frog", 16, 16,
            Anim("idle",    {0,1}, 2),
            Anim("shoot_d", {3},   3, false),
            Anim("shoot_r", {4},   3, false),
            Anim("shoot_l", {4},   3, false),
            Anim("shoot_u", {5},   3, false)), Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void Frog::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Frog, 1)

// ---------------------------------------------------------------------------
// Person — random direction walker with talk timer; slows player on contact
// ---------------------------------------------------------------------------
Person::Person(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("person", 16, 16,
            Anim("walk_d", {0,1}, 5),
            Anim("walk_r", {2,3}, 5),
            Anim("walk_u", {4,5}, 5),
            Anim("walk_l", {2,3}, 5)), Drawing::DrawOrder::ENTITIES)
{
    height = 4; width = 6;
    CenterOffset();
    offset.Y += 4.f;
    _talkTimer = 0.5f + (float)GlobalState::RNG.NextDouble();

    auto initFace = [&](Facing f) {
        facing = f;
        PlayFacing("walk");
        velocity = FacingDirection(f) * 10.f;
        if (velocity.X < 0.f) _flip = SpriteEffects::FlipHorizontally;
    };
    switch (p->Frame_) {
    case 0: initFace(Facing::DOWN);  break;
    case 1: initFace(Facing::RIGHT); break;
    case 2: initFace(Facing::UP);    break;
    case 3: initFace(Facing::LEFT);  break;
    default:
        // Case 4: random horizontal
        velocity.X = (GlobalState::RNG.Next(0,2) ? 1.f : -1.f) * 10.f;
        if (velocity.X > 0.f) Play("walk_r"); else Play("walk_l");
        if (velocity.X < 0.f) _flip = SpriteEffects::FlipHorizontally;
        _switchTimerMax = 0.5f;
        break;
    }
}
void Person::FaceRandom() {
    facing = static_cast<Facing>(GlobalState::RNG.Next(0, 4));
    PlayFacing("walk");
    velocity = FacingDirection(facing) * 10.f;
    _flip = (velocity.X < 0.f) ? SpriteEffects::FlipHorizontally : SpriteEffects::None;
}
void Person::Update() {
    Entity::Update();
    _switchTimer -= GameTimes::DeltaTime();
    if (_switchTimer < 0.f) {
        _switchTimer = _switchTimerMax;
        FaceRandom();
    }
    _talkTimer -= GameTimes::DeltaTime();
    if (_talkTimer < 0.f) {
        _talkTimer = 0.5f + (float)GlobalState::RNG.NextDouble();
        Sounds::SoundManager::PlaySoundEffect("talk_1","talk_1","talk_2","talk_3","talk_3");
    }
    PostUpdate();
}
void Person::Collided(Entity* o) {
    if (dynamic_cast<Player*>(o)) {
        Separate(o, this);
    }
    // Person does not damage or die
}

Rotator::Rotator(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("f_rotator", 16, 16,
            Anim("spin", {0,1}, 10)), Drawing::DrawOrder::BG_ENTITIES)
{ immovable = true; }
void Rotator::Update() { Entity::Update(); PostUpdate(); }
void Rotator::Collided(Entity* o) { Separate(o, this); }

SpikeRoller::SpikeRoller(EntityPreset* p, Player* pl)
    : Entity(p->Position,
        std::make_unique<AnimatedSpriteRenderer>("spike_roller_horizontal", 128, 16,
            Anim("spin", {0,1}, 5)), Drawing::DrawOrder::ROLLERS) {}
void SpikeRoller::Update() { Entity::Update(); PostUpdate(); }
void SpikeRoller::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o)) {
        p->ReceiveDamage(1, "SpikeRoller");
    }
}

WallBoss::WallBoss(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("wallboss_wall", 160, 32,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void WallBoss::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(WallBoss, 2)

// ===========================================================================
// Etc
// ===========================================================================
FollowerBro::FollowerBro(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("follower_bro", 16, 24,
            Anim("idle", {1,2}, 4)), Drawing::DrawOrder::ENTITIES) {}
void FollowerBro::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(FollowerBro, 1)

SageBoss::SageBoss(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("sage_boss", 16, 16,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void SageBoss::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(SageBoss, 2)

SpaceFace::SpaceFace(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("space_face", 16, 16,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void SpaceFace::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(SpaceFace, 2)

// ===========================================================================
// Go (Thorn bosses)
// ===========================================================================
BigThorn::BigThorn(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("briar_arm_left", 64, 80,
            Anim("off",    {0,1,2,3}, 4),
            Anim("active", {4,5},     5),
            Anim("hurt",   {7,8},     4, false)), Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void BigThorn::Update() { Entity::Update(); PostUpdate(); }
void BigThorn::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o)) {
        p->ReceiveDamage(1, "BigThorn");
    } else if (dynamic_cast<Broom*>(o) && CurAnimName() == "hurt") {
        Die();
    }
}

BlueThorn::BlueThorn(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("briar_arm_right", 64, 80,
            Anim("off",    {0,1,2,3}, 4),
            Anim("active", {4,5},     5),
            Anim("hurt",   {7,8},     4, false)), Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void BlueThorn::Update() { Entity::Update(); PostUpdate(); }
void BlueThorn::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o)) {
        p->ReceiveDamage(1, "BlueThorn");
    } else if (dynamic_cast<Broom*>(o) && CurAnimName() == "hurt") {
        Die();
    }
}

BriarBossBody::BriarBossBody(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("briar_boss", 16, 16,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void BriarBossBody::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(BriarBossBody, 1)

BriarBossFight::BriarBossFight(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("briar_boss", 16, 16,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void BriarBossFight::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(BriarBossFight, 1)

BriarBossMain::BriarBossMain(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("briar_boss_main", 32, 32,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void BriarBossMain::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(BriarBossMain, 2)

HappyThorn::HappyThorn(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("happy_thorn", 16, 16,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void HappyThorn::Update() { Entity::Update(); PostUpdate(); }
void HappyThorn::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o)) {
        p->ReceiveDamage(1, "HappyThorn");
    } else if (dynamic_cast<Broom*>(o)) {
        Die();
    }
}

// ===========================================================================
// Hotel
// ===========================================================================
EyeBossLandPhase::EyeBossLandPhase(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("eye_boss", 32, 32,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void EyeBossLandPhase::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(EyeBossLandPhase, 2)

EyeBossWaterPhase::EyeBossWaterPhase(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("eye_boss_water", 24, 24,
            Anim("idle", {0,1}, 6)), Drawing::DrawOrder::ENTITIES) {}
void EyeBossWaterPhase::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(EyeBossWaterPhase, 2)

Burst_Plant::Burst_Plant(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("burst_plant", 16, 16,
            Anim("idle", {0}, 8)), Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void Burst_Plant::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Burst_Plant, 1)

Dustmaid::Dustmaid(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("dustmaid", 16, 24,
            Anim("idle", {0}, 7)), Drawing::DrawOrder::ENTITIES) {}
void Dustmaid::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Dustmaid, 1)

SteamPipe::SteamPipe(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("steam_pipe", 16, 16,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void SteamPipe::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(SteamPipe, 1)

// ===========================================================================
// Redcave
// ===========================================================================
Four_Shooter::Four_Shooter(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("f_four_shooter", 16, 16,
            Anim("idle", {0,1}, 3)), Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void Four_Shooter::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Four_Shooter, 1)

Mover::Mover(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("f_mover", 16, 16,
            Anim("idle", {0,1}, 4)), Drawing::DrawOrder::ENTITIES) {}
void Mover::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Mover, 1)

OnOffLaser::OnOffLaser(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("on_off_shooter", 16, 16,
            Anim("idle", {0,1}, 2)), Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void OnOffLaser::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(OnOffLaser, 1)

Red_Boss::Red_Boss(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("red_boss", 32, 32,
            Anim("idle", {0}, 3)), Drawing::DrawOrder::ENTITIES) {}
void Red_Boss::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Red_Boss, 2)

Slasher::Slasher(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("f_slasher", 24, 24,
            Anim("idle", {0,1}, 3)), Drawing::DrawOrder::ENTITIES) {}
void Slasher::Update() { Entity::Update(); PostUpdate(); }
ENEMY_COLLIDE(Slasher, 1)

// ===========================================================================
// Suburb
// ===========================================================================

// ---------------------------------------------------------------------------
// SuburbKiller — faces player, approaches within 36px then pulses toward them
// ---------------------------------------------------------------------------
SuburbKiller::SuburbKiller(EntityPreset* p, Player* pl)
    : HealthDropper(p, p->Position,
        std::make_unique<AnimatedSpriteRenderer>("suburb_killer", 16, 16,
            Anim("idle_d", {0},    1),
            Anim("idle_r", {2},    1),
            Anim("idle_u", {4},    1),
            Anim("idle_l", {6},    1),
            Anim("walk_d", {0,1},  4),
            Anim("walk_r", {2,3},  4),
            Anim("walk_u", {4,5},  4),
            Anim("walk_l", {6,7},  4)), Drawing::DrawOrder::ENTITIES)
    , _pl(pl)
{
    width = height = 6;
    offset = {5.f, 5.f};
    Position += offset;
}
void SuburbKiller::Update() {
    Entity::Update();
    FaceTowards(_pl->Position);
    if (!_moving) {
        PlayFacing("idle");
        _moving = (Center() - _pl->Center()).Length() < 36.f;
    } else {
        PlayFacing("walk");
        _moveTimer += GameTimes::DeltaTime();
        if (_moveTimer > 0.5f) {
            _moveTimer = 0.f;
            Vector2 dir = _pl->Center() - Center();
            float len = dir.Length();
            if (len > 0.f) { dir.X /= len; dir.Y /= len; }
            velocity = dir * 30.f;
        }
    }
    PostUpdate();
}
void SuburbKiller::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o)) {
        p->ReceiveDamage(6, "SuburbKiller");
    } else if (dynamic_cast<Broom*>(o)) {
        GlobalState::SpawnEntity(new Explosion(Position));
        Die();
    }
}

#undef ENEMY_COLLIDE

} // namespace AnodyneSharp::Entities
