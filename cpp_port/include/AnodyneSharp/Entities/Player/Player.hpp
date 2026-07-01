#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/Rendering/SpriteRenderer.hpp"

namespace AnodyneSharp::Entities {

enum class PlayerState     { GROUND, AIR, AUTO_JUMP, ENTER_FALL, LADDER };
enum class PlayerAnimState { ANIM_DEFAULT, ANIM_ATK, ANIM_FALL, ANIM_DEAD,
                              as_idle, as_walk, as_slumped };

class Broom;
class Transformer;
class Foot_Overlay;
class PlayerReflection;
namespace Lights { class PlayerLight; }

class Player : public Entity {
public:
    static constexpr const char* Player_Sprite       = "young_player";
    static constexpr const char* Cell_Player_Sprite  = "young_player_cell";
    static constexpr const char* DrowningDamageDealer = "Drowned";
    static constexpr const char* TRANSITION_IDLE     = "idle_u";

    static constexpr int   DEFAULT_Y_OFFSET   = 4;
    static constexpr float action_latency_max = 0.24f;
    static constexpr float ATK_DELAY          = 0.2f;
    static constexpr float WATK_DELAY         = 0.35f;
    static constexpr float LATK_DELAY         = 0.4f;

    bool dontMove = false;

    PlayerAnimState ANIM_STATE = PlayerAnimState::as_idle;
    PlayerState     state      = PlayerState::GROUND;
    bool            invincible = false;

    Vector2 grid_entrance = {0,0};
    Vector2 additionalVel = {0,0};

    bool enter_fall = false;
    bool fall_smack = false;

    std::unique_ptr<Broom>           broom;
    std::unique_ptr<Transformer>     transformer;
    std::unique_ptr<Entity>          follower;
    std::unique_ptr<Foot_Overlay>    foot_overlay;
    std::unique_ptr<PlayerReflection> reflection;
    std::unique_ptr<Lights::PlayerLight> light;

    bool actions_disabled = false;
    bool skipBroom        = false;
    bool JustLanded       = false;
    bool reversed         = false;

    bool ON_CONVEYOR = false;

    Entity* raft = nullptr;

    static std::unique_ptr<AnimatedSpriteRenderer> GetSprite(bool cell);

    Player();
    ~Player();

    void Reset(bool fullReset = true);
    void ReceiveDamage(int amount, const std::string& dealer, bool knockback = true);

    void Update()     override;
    void PostUpdate() override;
    void Draw()       override;
    std::vector<Entity*> SubEntities() override;

    void Fall(Vector2 fallPoint) override;
    void Dash(Facing direction);
    void AutoJump(float time, Vector2 target, float speed = 10.f);
    void DontFall();
    void SlowTile()   override;
    void Puddle()     override;
    void Ladder()     override;
    void Reflection() override;
    void Grass()      override;
    void Conveyor(Touching direction) override;

private:
    static constexpr int   HITBOX_HEIGHT = 12;
    static constexpr int   HITBOX_WIDTH  = 10;
    static constexpr float jump_period   = 0.4f * 1.15f;
    static constexpr float INVINCIBLE_MAX = 0.8f;

    int   _walkSpeed = 70;
    float _invincibilityTime = 0.f;
    float _actionLatency = 0.f;
    float _bumpTimer = 0.f;
    float _fallTimer = 0.f;
    float _revTimer  = 0.f;
    float _stepNoiseTimer = 0.f;

    Vector2 _fallPoint = {0,0};
    Vector2 _dashState = {0,0};
    bool _onHole     = false;
    bool _isSlipping = false;
    bool _hasFallen  = false;
    bool _justFell   = false;
    bool _isSinking  = false;

    float _slowMul = 1.f;
    int   _slowTicks = 0;

    // Jump state
    float _jumpTimer    = 0.f;
    float _jumpPeriod   = jump_period;
    int   _jumpHeight   = 24;
    bool  _jumpHasTarget = false;
    Vector2 _jumpStart  = {0,0};
    Vector2 _jumpTarget = {0,0};

    void BeIdle();
    void DoMovement();
    void DoJump();
    void DoBroom();
    void UpdateFlicker();
    void GroundAnimation();
    void LadderLogic();
    void SlippingLogic();
    void ResetAfterFalling();
    void SinkingLogic();
    void DashLogic();
    void AirMovement();
    void SetInitVel(float mul = 1.f);
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::Player;
using AnodyneSharp::Entities::PlayerState;
using AnodyneSharp::Entities::PlayerAnimState;
