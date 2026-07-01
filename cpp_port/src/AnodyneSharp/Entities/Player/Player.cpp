// Player, Broom, Transformer implementations
#include "AnodyneSharp/MapData/Map.hpp"  // must come first to complete IPublicMap before GlobalState
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Entities/Player/Broom.hpp"
#include "AnodyneSharp/Entities/EventsAndLights.hpp"
#include "AnodyneSharp/Entities/Base/Shadow.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "AnodyneSharp/Input/KeyInput.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include "AnodyneSharp/GameEvents/Events.hpp"
#include "AnodyneSharp/Utilities/MathUtilities.hpp"
#include "AnodyneSharp/MapData/Map.hpp"
#define _USE_MATH_DEFINES
#include <cmath>

namespace AnodyneSharp::Entities {

Player::~Player() = default;

std::unique_ptr<AnimatedSpriteRenderer> Player::GetSprite(bool cell) {
    return std::make_unique<AnimatedSpriteRenderer>(
        cell ? Cell_Player_Sprite : Player_Sprite, 16, 16,
        Anim("walk_d",  {1,0},   6),
        Anim("walk_r",  {2,3},   8),
        Anim("walk_u",  {4,5},   6),
        Anim("walk_l",  {6,7},   8),
        Anim("attack_d",{8,9},   10, false),
        Anim("attack_r",{10,11}, 10, false),
        Anim("attack_u",{12,13}, 10, false),
        Anim("attack_l",{14,15}, 10, false),
        Anim("fall",    {28,29,30,31}, 5, false),
        Anim("slumped", {32},    1),
        Anim("whirl",   {25,26,27,24}, 12),
        Anim("idle_d",  {24},    4),
        Anim("idle_r",  {25},    4),
        Anim("idle_u",  {26},    4),
        Anim("idle_l",  {27},    4),
        Anim("idle_climb",{33},  1),
        Anim("climb",   {34,35}, 8),
        Anim("die",     {25,26,27,24,25,26,27,24,25,26,27,32}, 12, false)
    );
}

Player::Player()
    : Entity({0,0}, GetSprite(false), Drawing::DrawOrder::ENTITIES) {
    height = HITBOX_HEIGHT;
    width  = HITBOX_WIDTH;
    offset = {3.f, (float)DEFAULT_Y_OFFSET};
    Play("idle_u");
    ANIM_STATE = PlayerAnimState::as_idle;
    facing = Facing::UP;

    broom       = std::make_unique<Broom>(this);
    transformer = std::make_unique<Transformer>(this);
    shadow      = std::make_unique<Shadow>(this, Vector2{3.f,-1.f}, ShadowType::Normal, 20.f);
    foot_overlay = std::make_unique<Foot_Overlay>(this);
    reflection  = std::make_unique<PlayerReflection>(this);
    light       = std::make_unique<Lights::PlayerLight>(this);
}

void Player::Reset(bool fullReset) {
    if (fullReset) {
        BeIdle();
        broom->UpdateBroomType();
        broom->dust = nullptr;
        foot_overlay->OnMapChange();
        velocity = {0,0};
        state = PlayerState::GROUND;
        offset = {3.f,(float)DEFAULT_Y_OFFSET};
        raft = nullptr;
        follower = nullptr;
        broom->exists = false;
        actions_disabled = false;
        SetSolid(true);
        _isSlipping = false;
        _hasFallen  = false;
        _justFell   = false;
        _isSinking  = false;
        _dashState  = {0,0};
        JustLanded  = false;
        dontMove    = false;
        reversed    = false;
        _slowMul    = 1.f;
    }
    if (GlobalState::IsCell()) SetTexture(Cell_Player_Sprite,16,16);
    else SetTexture(Player_Sprite,16,16);
}

void Player::ReceiveDamage(int amount, const std::string& dealer, bool knockback) {
    if (invincible) return;
    GlobalState::CUR_HEALTH -= amount;
    GlobalState::DamageDealer = dealer;
    invincible = true;
    _invincibilityTime = INVINCIBLE_MAX;
    if (knockback) Flicker(INVINCIBLE_MAX);
    Sounds::SoundManager::PlaySoundEffect("player_hit_1", "player_hit_2");
}

void Player::BeIdle() { Play("idle_u"); ANIM_STATE = PlayerAnimState::as_idle; }

void Player::Update() {
    _onHole = false; // reset; tile collision will re-set if still on hole
    JustLanded = false;
    DoMovement();    // handles all states incl. DoBroom() for GROUND
    Entity::Update();
    if (broom) broom->Update();  // advance broom anim + set exists=false when done
}

void Player::PostUpdate() {
    if (invincible) {
        _invincibilityTime -= GameTimes::DeltaTime();
        if (_invincibilityTime <= 0) { invincible = false; Flicker(0); }
    }
    Entity::PostUpdate();
    if (broom) broom->PostUpdate();
}

void Player::Draw() {
    DrawImpl();
    if (broom) broom->Draw();
}

std::vector<Entity*> Player::SubEntities() {
    std::vector<Entity*> result;
    if (broom) {
        result.push_back(broom.get());
        for (auto& b : broom->SubEntities()) result.push_back(b);
    }
    if (foot_overlay) result.push_back(foot_overlay.get());
    if (reflection)   result.push_back(reflection.get());
    if (light)        result.push_back(light.get());
    if (shadow)       result.push_back(shadow.get());
    return result;
}

void Player::SetInitVel(float mul) {
    if (broom && broom->exists) { velocity = {0,0}; return; }
    using namespace Input;
    float speed = _walkSpeed * mul;
    velocity = {0,0};
    if (KeyInput::IsRebindableKeyPressed(KeyFunctions::Up))    { velocity.Y = -speed; facing = Facing::UP;    }
    if (KeyInput::IsRebindableKeyPressed(KeyFunctions::Down))  { velocity.Y =  speed; facing = Facing::DOWN;  }
    if (KeyInput::IsRebindableKeyPressed(KeyFunctions::Left))  { velocity.X = -speed; facing = Facing::LEFT;  }
    if (KeyInput::IsRebindableKeyPressed(KeyFunctions::Right)) { velocity.X =  speed; facing = Facing::RIGHT; }
    velocity.X *= _slowMul; velocity.Y *= _slowMul;
    // additionalVel applied after slowMul
    velocity += additionalVel;
}

void Player::DashLogic() {
    using namespace Input;
    if      (_dashState.X > 0 && !KeyInput::IsRebindableKeyPressed(KeyFunctions::Right)) _dashState.X = 0;
    else if (_dashState.X < 0 && !KeyInput::IsRebindableKeyPressed(KeyFunctions::Left))  _dashState.X = 0;
    else if (_dashState.X != 0) velocity.X = _walkSpeed * _dashState.X;

    if      (_dashState.Y > 0 && !KeyInput::IsRebindableKeyPressed(KeyFunctions::Down)) _dashState.Y = 0;
    else if (_dashState.Y < 0 && !KeyInput::IsRebindableKeyPressed(KeyFunctions::Up))   _dashState.Y = 0;
    else if (_dashState.Y != 0) velocity.Y = _walkSpeed * _dashState.Y;
}

void Player::AirMovement() {
    SetInitVel(0.83f);
    velocity += additionalVel;
}

void Player::SlippingLogic() {
    if (!_onHole) { _isSlipping = false; return; }
    _fallTimer -= GameTimes::DeltaTime();
    if (_fallTimer < 0.f) {
        Sounds::SoundManager::PlaySoundEffect("fall_in_hole");
        ANIM_STATE = PlayerAnimState::ANIM_FALL;
        _dashState = {0,0};
        _hasFallen  = true;
        _isSlipping = false;
        dontMove    = true;
        Position    = _fallPoint + Vector2{3.f, 5.f};
    }
}

void Player::ResetAfterFalling() {
    if (Frame() == 31) {
        Position    = grid_entrance;
        _hasFallen  = false;
        Flicker(1.f);
        Play("idle_d");
        SetSolid(false);
        _justFell   = true;
        dontMove    = false;
        ANIM_STATE  = PlayerAnimState::as_idle;
    }
}

void Player::SinkingLogic() {
    y_push += GameTimes::DeltaTime() * 16.f / 3.f;
    if (y_push > 16.f) {
        y_push = 0;
        Position = grid_entrance;
        ReceiveDamage(1, DrowningDamageDealer, false);
    }
}

void Player::LadderLogic() {
    using namespace Input;
    velocity = {0,0};
    if (!dontMove) {
        float lspeed = _walkSpeed * 0.7f;
        if      (KeyInput::IsRebindableKeyPressed(KeyFunctions::Up))   { velocity.Y = -lspeed; Play("climb"); }
        else if (KeyInput::IsRebindableKeyPressed(KeyFunctions::Down))  { velocity.Y =  lspeed; Play("climb"); }
        else                                                             Play("idle_climb");
        if (KeyInput::IsRebindableKeyPressed(KeyFunctions::Left))  velocity.X = -(float)_walkSpeed;
        if (KeyInput::IsRebindableKeyPressed(KeyFunctions::Right)) velocity.X =  (float)_walkSpeed;
        _stepNoiseTimer -= GameTimes::DeltaTime();
        if (_stepNoiseTimer < 0.f && velocity.Y != 0.f) {
            _stepNoiseTimer = 0.2f;
            Sounds::SoundManager::PlaySoundEffect("ladder_step_1", "ladder_step_2");
        }
    } else {
        Play("idle_climb");
    }
    // Reset to GROUND each frame; tile collision re-triggers Ladder() while on ladder
    state      = PlayerState::GROUND;
    ANIM_STATE = PlayerAnimState::as_idle;
}

void Player::GroundAnimation() {
    if (ANIM_STATE == PlayerAnimState::ANIM_ATK && broom && broom->AnimFinished()) {
        ANIM_STATE = PlayerAnimState::as_idle;
    }
    switch (ANIM_STATE) {
    case PlayerAnimState::ANIM_ATK:   PlayFacing("attack"); break;
    case PlayerAnimState::ANIM_FALL:  Play("fall");         break;
    case PlayerAnimState::ANIM_DEAD:                        break;
    case PlayerAnimState::as_slumped:
        Play("slumped");
        if (velocity.X != 0 || velocity.Y != 0) { ANIM_STATE = PlayerAnimState::as_walk; PlayFacing("walk"); }
        break;
    case PlayerAnimState::as_idle:
        if (velocity.X != 0 || velocity.Y != 0) { ANIM_STATE = PlayerAnimState::as_walk; PlayFacing("walk"); }
        else PlayFacing("idle");
        break;
    case PlayerAnimState::as_walk:
        if (velocity.X == 0 && velocity.Y == 0) { ANIM_STATE = PlayerAnimState::as_idle; PlayFacing("idle"); }
        else PlayFacing("walk");
        break;
    default: break;
    }
}

void Player::DoMovement() {
    // Reset per-frame state
    ON_CONVEYOR = false;
    additionalVel = {0,0};
    if (_slowTicks > 0 && --_slowTicks == 0) _slowMul = 1.f;

    switch (state) {
    case PlayerState::GROUND:
        if (shadow) shadow->visible = false;
        SetInitVel();
        if (dontMove) velocity = {0,0};
        if      (_isSlipping) SlippingLogic();
        else if (_hasFallen)  ResetAfterFalling();
        if (!dontMove && _isSinking) SinkingLogic();
        if (!_hasFallen && !actions_disabled && !GlobalState::InDeathRoom) DoBroom();
        GroundAnimation();
        break;

    case PlayerState::AIR: {
        if (dontMove) { velocity = {0,0}; }
        else          { AirMovement(); DashLogic(); }
        // Parabola: y_push = -height * sin(π * t/period)
        _jumpTimer += GameTimes::DeltaTime();
        float t = _jumpTimer / _jumpPeriod;
        y_push = (float)(-_jumpHeight * std::sin(3.14159265f * t));
        if (_jumpHasTarget) {
            Position.X = _jumpStart.X + (_jumpTarget.X - _jumpStart.X) * t;
            Position.Y = _jumpStart.Y + (_jumpTarget.Y - _jumpStart.Y) * t;
        }
        if (_jumpTimer >= _jumpPeriod) {
            // Land
            y_push = 0;
            if (_jumpHasTarget) Position = _jumpTarget;
            if (ON_CONVEYOR) Sounds::SoundManager::PlaySoundEffect("puddle_down");
            else             Sounds::SoundManager::PlaySoundEffect("player_jump_down");
            offset.Y = (float)DEFAULT_Y_OFFSET;
            JustLanded = true;
            state      = PlayerState::GROUND;
            SetSolid(true);
        }
        break;
    }
    case PlayerState::AUTO_JUMP: {
        _jumpTimer += GameTimes::DeltaTime();
        float t = _jumpTimer / _jumpPeriod;
        y_push = (float)(-_jumpHeight * std::sin(3.14159265f * t));
        if (_jumpHasTarget) {
            Position.X = _jumpStart.X + (_jumpTarget.X - _jumpStart.X) * t;
            Position.Y = _jumpStart.Y + (_jumpTarget.Y - _jumpStart.Y) * t;
        }
        if (_jumpTimer >= _jumpPeriod) {
            y_push = 0;
            if (_jumpHasTarget) Position = _jumpTarget;
            state = PlayerState::GROUND;
        }
        break;
    }
    case PlayerState::ENTER_FALL:
        velocity = {0,0};
        if (AnodyneSharp::MathUtilities::MoveTo(offset.Y, 0.f, 102.f) && fall_smack) {
            state      = PlayerState::GROUND;
            Play("slumped");
            ANIM_STATE = PlayerAnimState::as_slumped;
            fall_smack = false;
            Sounds::SoundManager::PlaySoundEffect("hit_ground_1");
        }
        break;

    case PlayerState::LADDER:
        LadderLogic();
        break;
    }
}

void Player::DoJump() {
    // Called from DoBroom context — handled inside DoMovement's GROUND case
}

void Player::DoBroom() {
    if (state == PlayerState::AIR) return;
    if (_actionLatency > 0) {
        _actionLatency -= GameTimes::DeltaTime();
    }
    using namespace Input;
    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) && _actionLatency <= 0 && !skipBroom) {
        if (!broom->exists) {
            broom->Attack();
            _actionLatency = action_latency_max;
            ANIM_STATE = PlayerAnimState::ANIM_ATK;
        }
    }
    // Jump with Cancel key
    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel)
            && !_isSinking && GlobalState::inventory.CanJump) {
        state       = PlayerState::AIR;
        _jumpTimer  = 0.f;
        _jumpPeriod = jump_period;
        _jumpHeight = 24;
        _jumpHasTarget = false;
        _jumpStart  = Position;
        if (shadow) shadow->visible = true;
        if (broom)  broom->exists   = false;
        _isSlipping = false;
        if (ON_CONVEYOR) Sounds::SoundManager::PlaySoundEffect("puddle_up");
        else             Sounds::SoundManager::PlaySoundEffect("player_jump_up");
    }
    skipBroom = false;
}
void Player::UpdateFlicker() {}

void Player::Fall(Vector2 fp) {
    if (state != PlayerState::AIR && !_isSlipping && !_hasFallen && !GlobalState::FUCK_IT_MODE_ON) {
        _isSlipping = true;
        // Coyote time: if just landed give no grace, otherwise ~0.13 s
        _fallTimer = JustLanded ? -1.f : (0.016f * 8.f + 0.001f);
    }
    _fallPoint = fp;
    _onHole    = true;
}
void Player::DontFall() { _isSlipping = false; }
void Player::SlowTile() { _slowMul = 0.5f; _slowTicks = 3; }
void Player::Puddle()   { ON_CONVEYOR = true; if (foot_overlay) foot_overlay->Puddle(); }
void Player::Reflection() { if (reflection) reflection->Reflection(); }
void Player::Ladder()   {
    if (state == PlayerState::GROUND) state = PlayerState::LADDER;
}
void Player::Grass()    { if (foot_overlay) foot_overlay->Grass(); }
void Player::Conveyor(Touching direction) {
    ON_CONVEYOR = true;
    const float speed = 35.f;
    additionalVel += Entity::FacingDirection(Entity::FacingFromTouching(direction)) * speed;
    if (!_isSinking) { _slowMul = 0.5f; _isSinking = true; }
}
void Player::Dash(Facing direction) {
    auto set = [](float& v, float add) {
        if (v == 0)               { v = add * 1.3f;  Sounds::SoundManager::PlaySoundEffect("dash_pad_1"); }
        else if (v * add < 0)     { v = 0; }
        else if (std::abs(v)<1.5f){ v = add * 1.77f; Sounds::SoundManager::PlaySoundEffect("dash_pad_2"); }
    };
    Vector2 d = FacingDirection(direction);
    if (d.X != 0) set(_dashState.X, d.X);
    else          set(_dashState.Y, d.Y);
}
void Player::AutoJump(float time, Vector2 target, float /*speed*/) {
    _jumpTimer     = 0.f;
    _jumpPeriod    = time;
    _jumpHeight    = 24;
    _jumpHasTarget = true;
    _jumpStart     = Position;
    _jumpTarget    = target;
    SetSolid(false);
    state    = PlayerState::AUTO_JUMP;
    velocity = {0,0};
}

// Broom
Broom::Broom(Player* player)
    : Entity(player->Position,
             std::make_unique<AnimatedSpriteRenderer>("broom", 16, 16,
                 Anim("stab", std::vector<int>{1, 2, 2, 1, 0, 0}, 20, false)),
             Drawing::DrawOrder::FG_SPRITES), _player(player) {
    exists = false;
    immovable = true;
}

void Broom::Attack() {
    exists = true;
    facing = _player->facing;
    Play("stab", true);
    Sounds::SoundManager::PlaySoundEffect("swing_broom_1", "swing_broom_2", "swing_broom_3");
    UpdatePos();
    if (GlobalState::FireEvent) GlobalState::FireEvent(new GameEvents::BroomUsed());
}

void Broom::UpdatePos() {
    facing = _player->facing;
    switch (_player->facing) {
    case Facing::LEFT:
        rotation = 0.f;
        Position = {_player->Position.X - 14.f, _player->Position.Y};
        switch (Frame()) { case 0: Position.X += 10.f; break; case 1: Position.X += 6.f; break; case 2: Position.X -= 1.f; break; }
        break;
    case Facing::RIGHT:
        rotation = 3.14159265f;
        Position = {_player->Position.X + (float)_player->width, _player->Position.Y - 2.f};
        switch (Frame()) { case 0: Position.X -= 12.f; break; case 1: Position.X -= 8.f; break; case 2: Position.X -= 1.f; break; }
        break;
    case Facing::UP:
        rotation = 3.14159265f * 0.5f;
        Position = {_player->Position.X - 2.f, _player->Position.Y - 16.f};
        switch (Frame()) { case 0: Position.Y += 12.f; break; case 1: Position.Y += 6.f; break; case 2: Position.Y += 2.f; break; }
        break;
    case Facing::DOWN:
        rotation = 3.14159265f * 1.5f;
        Position = {_player->Position.X - 6.f, _player->Position.Y + (float)_player->height};
        switch (Frame()) { case 0: Position.Y -= 8.f; break; case 1: Position.Y -= 5.f; break; case 2: Position.Y -= 2.f; break; }
        break;
    }
}

void Broom::UpdateBroomType() {
    Position = {_player->Position.X - 10.f, _player->Position.Y};
    width  = 16;
    height = 16;
    offset = {0.f, 0.f};
    if      (GlobalState::IsCell())  SetTexture("broom_cell", 16, 16);
    else if (GlobalState::IsKnife()) SetTexture("knife",      16, 16);
    else                             SetTexture("broom",      16, 16);
}
void Broom::Use(Facing direction) { facing = direction; Attack(); }
void Broom::Update() {
    if (!exists) return;
    if (AnimFinished()) {
        exists = false;
    } else {
        UpdatePos();
    }
    Entity::Update();
}
void Broom::Draw() { if (exists) DrawImpl(); }
std::vector<Entity*> Broom::SubEntities() {
    if (dust) return {dust};
    return {};
}

// Transformer
Transformer::Transformer(Player* player)
    : Entity(player->Position, Drawing::DrawOrder::FG_SPRITES), _player(player) {
    exists = false;
    visible = false;
    // Selector (animated) and selected_tile (static, shows chosen tile)
    _selector = std::make_unique<Entity>(
        Vector2{0,0},
        std::make_unique<AnimatedSpriteRenderer>("selector", 16, 16,
            Anim("a", std::vector<int>{0,1}, 4)),
        Drawing::DrawOrder::ENTITIES);
    _selector->exists = false;

    _selectedTile = std::make_unique<Entity>(
        Vector2{0,0},
        std::make_unique<StaticSpriteRenderer>("selector", 16, 16, 1),
        Drawing::DrawOrder::BG_ENTITIES);
    _selectedTile->exists = false;
}

void Transformer::Reset() {
    if (_selector)      _selector->exists      = false;
    if (_selectedTile)  _selectedTile->exists  = false;
}

void Transformer::OnAction() {
    if (!_selector || !GlobalState::Map) return;
    MapData::IPublicMap* imap = GlobalState::Map;
    // Position selector in front of player
    _selector->Position = Vector2{
        (float)((int)((_player->Center().X / 16.f) + Entity::FacingDirection(_player->facing).X)) * 16.f,
        (float)((int)((_player->Center().Y / 16.f) + Entity::FacingDirection(_player->facing).Y)) * 16.f
    };

    if (!_selector->exists) {
        _selector->exists = true;
    } else if (!_selectedTile->exists) {
        _selectedTile->Position = _selector->Position;
        _selectedTile->exists   = true;
        Sounds::SoundManager::PlaySoundEffect("menu_move");
    } else {
        // Swap tiles
        Point p1 = imap->ToMapLoc(_selector->Center());
        Point p2 = imap->ToMapLoc(_selectedTile->Center());
        int t1 = imap->GetTile(MapData::Layer::BG, p1);
        int t2 = imap->GetTile(MapData::Layer::BG, p2);
        imap->ChangeTile(MapData::Layer::BG, p1, t2);
        imap->ChangeTile(MapData::Layer::BG, p2, t1);
        _selector->exists      = false;
        _selectedTile->exists  = false;
        Sounds::SoundManager::PlaySoundEffect("menu_select");
        if (GlobalState::FireEvent)
            GlobalState::FireEvent(new GameEvents::StartScreenTransition());
    }
}

void Transformer::Update() {
    if (!_player) return;
    // Keep selector positioned in front of player
    if (_selector && _selector->exists) {
        Vector2 d = Entity::FacingDirection(_player->facing);
        _selector->Position = Vector2{
            (float)((int)(_player->Center().X / 16.f + d.X)) * 16.f,
            (float)((int)(_player->Center().Y / 16.f + d.Y)) * 16.f
        };
        _selector->Update();
    }
    if (_selectedTile) _selectedTile->Update();
    PostUpdate();
}

void Transformer::Draw() {
    if (_selector)      _selector->Draw();
    if (_selectedTile)  _selectedTile->Draw();
}

std::vector<Entity*> Transformer::SubEntities() {
    std::vector<Entity*> result;
    if (_selector)     result.push_back(_selector.get());
    if (_selectedTile) result.push_back(_selectedTile.get());
    return result;
}

} // namespace AnodyneSharp::Entities
