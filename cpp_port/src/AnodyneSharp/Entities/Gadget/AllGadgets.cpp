// Real implementations for Gadget entities
#include "AnodyneSharp/Entities/Gadget/AllGadgets.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Entities/EntityManager.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Registry/InventoryManager.hpp"
#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Utilities/MathUtilities.hpp"
#include "AnodyneSharp/Entities/Player/Broom.hpp"
#include "AnodyneSharp/Dialogue/DialogueManager.hpp"

namespace AnodyneSharp::Entities {

// ===========================================================================
// BaseTreasure
// ===========================================================================
void BaseTreasure::GetTreasure() {
    Sounds::SoundManager::PlaySoundEffect("gettreasure");
    if (_dialogueID >= 0)
        GlobalState::SetDialogue(Dialogue::DialogueManager::GetDialogue("misc","any","treasure",_dialogueID));
}

// ===========================================================================
// Blocker
// ===========================================================================
Blocker::Blocker(EntityPreset* p, Player* pl)
    : Entity(p->Position, Drawing::DrawOrder::ENTITIES), _pl(pl)
{ width=64; height=4; visible=false; immovable=true; }
void Blocker::Update() {
    Entity::Update();
    if (_pl && Hitbox().Intersects(_pl->Hitbox())) {
        if (_pl->velocity.Y > 0 && _pl->Position.Y < Position.Y)
            _pl->Position.Y = Position.Y - _pl->height;
        else if (_pl->velocity.Y < 0 && _pl->Position.Y > Position.Y)
            _pl->Position.Y = Position.Y + height;
    }
    PostUpdate();
}

// ===========================================================================
// Button
// ===========================================================================
Button::Button(EntityPreset* p, Player* /*pl*/)
    : Entity(p->Position, "buttons", 16, 16, Drawing::DrawOrder::BG_ENTITIES)
{
    _permanent = (p->Frame_ == 0);
    const std::string& m = GlobalState::CURRENT_MAP_NAME;
    int f = (m=="STREET") ? 6 : (m=="BEDROOM") ? 0 : (m=="REDCAVE") ? 4 : (m=="CELL") ? 8 : 2;
    SetFrame(f);
    immovable = true;
}
void Button::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o)) _pressed |= (p->state == PlayerState::GROUND);
    else _pressed = true;
}
void Button::Update() {
    if (_pressed && !_incremented) {
        _incremented=true; GlobalState::PUZZLES_SOLVED++; SetFrame(Frame()+1);
        Sounds::SoundManager::PlaySoundEffect("button_down");
    } else if (!_pressed && _incremented && !_permanent) {
        GlobalState::PUZZLES_SOLVED--; _incremented=false; SetFrame(Frame()-1);
        Sounds::SoundManager::PlaySoundEffect("button_up");
    }
    _pressed = false;
    PostUpdate();
}

// ===========================================================================
// Console
// ===========================================================================
Console::Console(EntityPreset* p, Player* /*pl*/)
    : Entity(p->Position, "console", 16, 16, Drawing::DrawOrder::ENTITIES), _preset(p)
{
    immovable=true;
    if (p->GetActivated()) { Play("green"); GlobalState::PUZZLES_SOLVED++; }
    else Play("active");
}
void Console::Update() { PostUpdate(); }
void Console::Collided(Entity* o) { Separate(o, this); }
bool Console::PlayerInteraction(Facing /*d*/) {
    if (_preset->GetActivated()) return false;
    _preset->SetActivated(true); Play("green"); GlobalState::PUZZLES_SOLVED++;
    Sounds::SoundManager::PlaySoundEffect("get_small_health");
    return true;
}

// ===========================================================================
// DashPad
// ===========================================================================
DashPad::DashPad(EntityPreset* p, Player* /*pl*/)
    : Entity(p->Position, "dash_pads", 16, 16, Drawing::DrawOrder::BG_ENTITIES)
{
    SetFrame(p->Frame_ + 4 + (GlobalState::IsCell() ? 4 : 0));
    facing = (p->Frame_==0) ? Facing::UP : (p->Frame_==1) ? Facing::RIGHT : (p->Frame_==2) ? Facing::DOWN : Facing::LEFT;
}
void DashPad::Update() { Entity::Update(); _t += GameTimes::DeltaTime(); if(_t>1.f){opacity=1.f;_t=0.f;} PostUpdate(); }
void DashPad::Collided(Entity* o) {
    if (auto* p = dynamic_cast<Player*>(o))
        if (opacity==1.f && p->state==PlayerState::GROUND && Hitbox().Contains(p->Center()))
            { opacity=0.5f; _t=0.f; p->Dash(facing); }
}

// ===========================================================================
// Door
// ===========================================================================
Door::Door(EntityPreset* p, Player* pl)
    : Entity(p->Position, Drawing::DrawOrder::ENTITIES), _pl(pl)
{ width=16; height=16; _linked=EntityManager::GetLinkedDoor(p); immovable=true; _onDoor=pl->Hitbox().Intersects(Hitbox()); }
void Door::Update() { Entity::Update(); if(!_pl->Hitbox().Intersects(Hitbox())) _onDoor=false; PostUpdate(); }
void Door::Collided(Entity* o) { if(dynamic_cast<Player*>(o) && Active && !_onDoor) TeleportPlayer(); }
void Door::TeleportPlayer() {
    if (_linked) {
        GlobalState::NEXT_MAP_NAME = _linked->Map;
        GlobalState::PLAYER_WARP_TARGET = _linked->Door->Position + teleportOffset;
        GlobalState::WARP = true;
    }
    _onDoor=true;
    if (!_sfx.empty()) Sounds::SoundManager::PlaySoundEffect(_sfx);
}
BlankPortal::BlankPortal(EntityPreset*p,Player*pl):Door(p,pl){
    auto l = std::make_unique<Layer>(Drawing::DrawOrder::BG_ENTITIES, this);
    sprite = std::make_unique<AnimatedSpriteRenderer>("whiteportal", 16, 16, l.release(),
        std::vector<Anim>{Anim("a", GlobalState::IsCell() ? std::vector<int>{4,5} : std::vector<int>{0,1,2}, 8)});
    Play("a");
    width = height = 2;
    CenterOffset();
    if (GlobalState::IsCell()) teleportOffset = {0.f, -12.f};
    _sfx = "teleport_up";
}
DirectionalDoor::DirectionalDoor(EntityPreset*p,Player*pl):Door(p,pl){visible=false;}
void DirectionalDoor::Collided(Entity*o){if(!_onDoor&&Active)TeleportPlayer();(void)o;}
void DirectionalDoor::TeleportPlayer(){GlobalState::NewMapFacing=_exitDir;Door::TeleportPlayer();}
FallDoor::FallDoor(EntityPreset*p,Player*pl):Door(p,pl){visible=false;}
void FallDoor::Collided(Entity*o){
    if(auto*p=dynamic_cast<Player*>(o);p&&Active){
        if(p->state==PlayerState::GROUND||p->state==PlayerState::ENTER_FALL)
            {TeleportPlayer();p->enter_fall=true;if(_smack){p->fall_smack=true;p->state=PlayerState::GROUND;p->dontMove=true;}Active=false;}
        else if(p->state==PlayerState::AUTO_JUMP) _smack=false;
    }
}
NexusDoor::NexusDoor(EntityPreset*p,Player*pl):Door(p,pl){visible=false;}
NexusPad::NexusPad(EntityPreset*p,Player*pl):Door(p,pl){_sfx="";GlobalState::events.IncEvent("ActivatedNexusPad");}
void NexusPad::Update(){Door::Update();if(!_onPad&&!_pl->Hitbox().Intersects(Hitbox())){_onPad=false;Play("off");}}
void NexusPad::Collided(Entity*o){if(!_onPad&&dynamic_cast<Player*>(o)){Play("on");Sounds::SoundManager::PlaySoundEffect("menu_select");_onPad=true;}}
bool NexusPad::PlayerInteraction(Facing){TeleportPlayer();return true;}
NoMoveDoor::NoMoveDoor(EntityPreset*p,Player*pl):Door(p,pl){visible=false;}
OneWayDoor::OneWayDoor(EntityPreset*p,Player*pl):Door(p,pl){exists=false;}
WhirlPool::WhirlPool(EntityPreset*p,Player*pl):Door(p,pl),_preset(p){
    _sfx="";
    if(GlobalState::events.GetEvent("fisherman.dead")!=0) Play("whirl_red"); else Play("whirl");
    if(GlobalState::CURRENT_MAP_NAME=="REDSEA") teleportOffset={0.f,-36.f};
}
void WhirlPool::Update(){Entity::Update();if(AnimFinished()){Play("whirl_red");}PostUpdate();}
void WhirlPool::DoTransition(){GlobalState::events.IncEvent("fisherman.dead");Play("transition");}

// ===========================================================================
// Dust
// ===========================================================================
Dust::Dust(Vector2 pos, bool isRaft)
    : Entity(pos, "dust", 16, 16, Drawing::DrawOrder::BG_ENTITIES), IsRaft(isRaft)
{ Play("poofed"); }
void Dust::Collided(Entity*) {}
void Dust::Update() {
    Entity::Update();
    velocity={0,0}; ON_CONVEYOR=false;
    if(AnimFinished()&&(CurAnimName()=="fallpoof"||CurAnimName()=="poof")) exists=false;
    PostUpdate();
}

// ===========================================================================
// Explosion
// ===========================================================================
Explosion::Explosion(Vector2 pos)
    : Entity(pos, "enemy_explode_2", 24, 24, Drawing::DrawOrder::BG_ENTITIES)
{ Play("explode"); Sounds::SoundManager::PlaySoundEffect("hit_wall"); }
void Explosion::Update(){Entity::Update();if(AnimFinished())exists=false;PostUpdate();}

// ===========================================================================
// Gate
// ===========================================================================
bool Gate::ConditionSatisfied() const {
    int v = ((_preset->Frame_%10)>=5) ? GlobalState::PUZZLES_SOLVED : GlobalState::ENEMIES_KILLED;
    return v > (_preset->Frame_%5);
}
Gate::Gate(EntityPreset*p,Player*pl)
    : Entity(p->Position,"gates",16,16,Drawing::DrawOrder::ENTITIES), _preset(p), _pl(pl)
{
    immovable=true;
    _heldDown=pl->Hitbox().Intersects(Hitbox());
    if(_heldDown&&!ConditionSatisfied()){Play("opened");SetSolid(false);}else Play("still");
}
void Gate::Collided(Entity*o){if(Solid())Separate(this,o);}
void Gate::Update(){
    Entity::Update();
    if(_heldDown){
        if(!_pl->Hitbox().Intersects(Hitbox())){_heldDown=false;Play("close");Sounds::SoundManager::PlaySoundEffect("hit_ground_1");SetSolid(true);}
    } else if(CurAnimName()=="still"||CurAnimName()=="close"){
        if(ConditionSatisfied()){
            Play("open");Sounds::SoundManager::PlaySoundEffect("open");
            if(_preset->Frame_<10)_preset->SetAlive(false);
            SetSolid(false);
        }
    }
    PostUpdate();
}
BigCardGate::BigCardGate(EntityPreset*p,Player*pl):Gate(p,pl){}
BigGate::BigGate(EntityPreset*p,Player*pl):Gate(p,pl){}
BigKeyGate::BigKeyGate(EntityPreset*p,Player*pl):Gate(p,pl){}
KeyBlockSentinel::KeyBlockSentinel(EntityPreset*p,Player*):Entity(p->Position,Drawing::DrawOrder::ENTITIES){visible=false;}
KeyCardGate::KeyCardGate(EntityPreset*p,Player*pl):Gate(p,pl){}
SmallKeyGate::SmallKeyGate(EntityPreset*p,Player*pl):Gate(p,pl){}

// ===========================================================================
// Detectors
// ===========================================================================
GoDetector::GoDetector(EntityPreset*p,Player*):Entity(p->Position,Drawing::DrawOrder::ENTITIES),_preset(p){visible=false;}
void GoDetector::Update(){Entity::Update();PostUpdate();}

GoHappyBlocker::GoHappyBlocker(EntityPreset*p,Player*)
    : Entity(p->Position,"briar_ground_thorn",16,16,Drawing::DrawOrder::ENTITIES)
{
    if (GlobalState::events.GetEvent("BlueDone")==1) { exists=false;return; }
    immovable=true; width=20; height=20; Position.Y+=2;
}
void GoHappyBlocker::Collided(Entity*o){Separate(this,o);}

GoQuestDoorBlocker::GoQuestDoorBlocker(EntityPreset*p,Player*):Entity(p->Position,Drawing::DrawOrder::ENTITIES){visible=false;exists=false;}

// ===========================================================================
// HealthEntity
// ===========================================================================
HealthEntity::HealthEntity(EntityPreset*p,Player*)
    : Entity(p->Position,"health_pickups",16,16,Drawing::DrawOrder::ENTITIES), _preset(p), _isLarge(p->Frame_==0)
{ SetFrame(_isLarge?0:2); }
void HealthEntity::Update(){Entity::Update();PostUpdate();}
void HealthEntity::Collided(Entity*o){
    if(dynamic_cast<Player*>(o)){
        int heal=_isLarge?4:1;
        GlobalState::CUR_HEALTH=std::min(GlobalState::CUR_HEALTH+heal,GlobalState::MAX_HEALTH_get());
        Sounds::SoundManager::PlaySoundEffect("get_small_health");
        _preset->SetAlive(false); exists=false;
    }
}

// ===========================================================================
// Holes
// ===========================================================================
Hole::Hole(EntityPreset*p,Player*):Entity(p->Position,"hole",16,16,Drawing::DrawOrder::MAP_BG){immovable=true;SetSolid(false);}
void Hole::Collided(Entity*o){if(auto*p=dynamic_cast<Player*>(o);p&&p->state!=PlayerState::AIR)p->Fall(Position);}

CrackedTile::CrackedTile(EntityPreset*p,Player*):Entity(p->Position,"crackedtiles",16,16,Drawing::DrawOrder::MAP_BG)
{immovable=true;SetSolid(false);SetFrame(p->Frame_);}
void CrackedTile::Collided(Entity*o){
    if(auto*p=dynamic_cast<Player*>(o);p&&p->state!=PlayerState::AIR){
        _t-=GameTimes::DeltaTime();
        if(_t<0){Sounds::SoundManager::PlaySoundEffect("floor_crack");exists=false;}
    }
}

// ===========================================================================
// JumpTrigger
// ===========================================================================
JumpTrigger::JumpTrigger(EntityPreset*p,Player*):Entity(p->Position,Drawing::DrawOrder::ENTITIES){SetSolid(false);}

// ===========================================================================
// Key
// ===========================================================================
Key::Key(EntityPreset*p,Player*)
    : Entity(p->Position,"key",16,16,Drawing::DrawOrder::ENTITIES), _preset(p)
{
    SetFrame(p->Frame_);
    if(p->TypeValue=="boss_rush"){_bossRush=true;visible=(GlobalState::ENEMIES_KILLED>1);}
}
void Key::Update(){Entity::Update();if(!visible&&_bossRush&&GlobalState::ENEMIES_KILLED>0)visible=true;PostUpdate();}
void Key::Collided(Entity*){
    if(!visible)return;
    visible=false; GlobalState::inventory.AddCurrentMapKey();
    Sounds::SoundManager::PlaySoundEffect("keyget");
    if(_preset){_preset->SetAlive(false);exists=false;}
}

// ===========================================================================
// Propelled
// ===========================================================================
Propelled::Propelled(Vector2 pos,Vector2 vel,float):Entity(pos,Drawing::DrawOrder::FG_SPRITES){velocity=vel;}
void Propelled::Update(){Entity::Update();PostUpdate();}

// ===========================================================================
// SoundTestConsole
// ===========================================================================
SoundTestConsole::SoundTestConsole(EntityPreset*p,Player*):Entity(p->Position,"console",16,16,Drawing::DrawOrder::ENTITIES){immovable=true;}
bool SoundTestConsole::PlayerInteraction(Facing){return false;}

// ===========================================================================
// SpringPad
// ===========================================================================
SpringPad::SpringPad(EntityPreset*p,Player*)
    : Entity(p->Position,"spring_pad",16,16,Drawing::DrawOrder::BG_ENTITIES)
{
    immovable=true;
    switch(p->Frame_){case 1:_dist=48;_jumpT=0.5f;break;case 2:_dist=64;_jumpT=0.7f;break;default:_dist=32;_jumpT=0.3f;break;}
    Play("still");
}
void SpringPad::Collided(Entity*o){
    Entity::Collided(o);
    if(auto*p=dynamic_cast<Player*>(o)){
        _playerCol=true;
        if(p->JustLanded){
            _activated=true; Sounds::SoundManager::PlaySoundEffect("spring_bounce"); Play("wobble");
            p->facing=Facing::DOWN; p->AutoJump(_jumpT,p->Position+Vector2{0.f,(float)_dist},10.f);
        } else {
            if(!_activated&&p->state==PlayerState::GROUND) Play("pressed");
            else if(p->state==PlayerState::AIR) Play("still");
        }
    }
}
void SpringPad::Update(){Entity::Update();if((_activated&&AnimFinished())||!_playerCol){_activated=false;Play("still");}_playerCol=false;PostUpdate();}

// ===========================================================================
// PillarSwitch
// ===========================================================================
PillarSwitch::PillarSwitch(EntityPreset*p,Player*):Entity(p->Position,"pillar_switch",16,16,Drawing::DrawOrder::ENTITIES)
{immovable=true;Play(GlobalState::PillarSwitchOn==0?"up":"down");}
void PillarSwitch::Update(){
    Entity::Update();
    if(_hitTm>0){_hitTm-=GameTimes::DeltaTime();if(_hitTm<=0)Play(GlobalState::PillarSwitchOn==0?"up":"down");}
    PostUpdate();
}
void PillarSwitch::Collided(Entity*o){
    Entity::Collided(o); Separate(this,o);
    if(_hitTm<=0) {
        // Check if o is a Broom by IsDust flag (Broom::IsDust is set on Broom entities)
        auto* br = dynamic_cast<Broom*>(o);
        if(br) {
            Play("hit");_hitTm=1.f;Sounds::SoundManager::PlaySoundEffect("broom_hit");
            GlobalState::PillarSwitchOn=1-GlobalState::PillarSwitchOn;
        }
    }
}

// ===========================================================================
// SwitchPillar
// ===========================================================================
int SwitchPillar::TargetFrame() const { return std::abs(_defFrame-GlobalState::PillarSwitchOn); }
SwitchPillar::SwitchPillar(EntityPreset*p,Player*):Entity(p->Position,"dame-switch-pillar",16,16,Drawing::DrawOrder::BG_ENTITIES)
{immovable=true;_defFrame=1-p->Frame_;_cur=TargetFrame();Play(_cur==UP_FRAME?"up":"down");}
void SwitchPillar::Update(){
    Entity::Update();
    int t=TargetFrame();
    if(t!=_cur){
        if(_cur==DOWN_FRAME){Sounds::SoundManager::PlaySoundEffect("dash_pad_2");Play("solidify");}
        else{Sounds::SoundManager::PlaySoundEffect("dash_pad_1");Play("dissolve");}
        _cur=t;
    }
    PostUpdate();
}
void SwitchPillar::Collided(Entity*o){Entity::Collided(o);if(Frame()==UP_FRAME)Separate(this,o);}

// ===========================================================================
// TreasureChest
// ===========================================================================
void TreasureChest::SetTreasure(){
    int f=_preset->Frame_;
    if(f==0||f==4||f==5||f==6) _treasure=std::make_unique<BroomTreasure>(_preset,nullptr);
    else if(f==1)              _treasure=std::make_unique<KeyTreasure>(_preset,nullptr);
    else if(f>=7&&f<=20)       _treasure=std::make_unique<SecretTreasure>(_preset,nullptr);
    else                       _treasure=std::make_unique<Treasure>(Position);
}
TreasureChest::TreasureChest(EntityPreset*p,Player*)
    : Entity(p->Position,"treasureboxes",16,16,Drawing::DrawOrder::ENTITIES), _preset(p)
{
    immovable=true;
    int f=GlobalState::IsCell()?4:0;
    if(p->GetActivated()){f++;opened=true;}else SetTreasure();
    SetFrame(f);
}
void TreasureChest::Update(){Entity::Update();if(opened&&_treasure)_treasure->Update();PostUpdate();}
bool TreasureChest::PlayerInteraction(Facing d){
    if(opened||d!=Facing::UP)return false;
    opened=true;_treasure->GetTreasure();SetFrame(Frame()+1);_preset->SetActivated(true);return true;
}
void TreasureChest::Collided(Entity*o){Separate(this,o);}

// ===========================================================================
// Treasure flyup
// ===========================================================================
Treasure::Treasure(Vector2 pos):BaseTreasure(pos,Drawing::DrawOrder::FG_SPRITES),_end{pos.X,pos.Y-16.f}
{exists=false;_dialogueID=-1;}
void Treasure::Update(){
    Entity::Update();
    if(_flickering){if(Position.Y>_end.Y)MathUtilities::MoveTo(Position.Y,_end.Y,30.f);}else exists=false;
    PostUpdate();
}
void Treasure::GetTreasure(){BaseTreasure::GetTreasure();Flicker(1.f);exists=true;}

// ===========================================================================
// Specific treasures
// ===========================================================================
BootsTreasure::BootsTreasure(EntityPreset*p,Player*):BaseTreasure(p->Position,Drawing::DrawOrder::FG_SPRITES){_dialogueID=4;}
void BootsTreasure::GetTreasure(){BaseTreasure::GetTreasure();GlobalState::inventory.CanJump=true;}

BroomTreasure::BroomTreasure(EntityPreset*p,Player*):BaseTreasure(p->Position,Drawing::DrawOrder::FG_SPRITES){_dialogueID=1;}
void BroomTreasure::GetTreasure(){BaseTreasure::GetTreasure();GlobalState::inventory.HasBroom=true;if(GlobalState::inventory.EquippedBroom()==BroomType::NONE)GlobalState::inventory.SetEquippedBroom(BroomType::Normal);}

CardTreasure::CardTreasure(EntityPreset*p,Player*):BaseTreasure(p->Position,Drawing::DrawOrder::FG_SPRITES){_dialogueID=-1;}
void CardTreasure::GetTreasure(){BaseTreasure::GetTreasure();}

KeyTreasure::KeyTreasure(EntityPreset*p,Player*):BaseTreasure(p->Position,Drawing::DrawOrder::FG_SPRITES){_dialogueID=2;}
void KeyTreasure::GetTreasure(){BaseTreasure::GetTreasure();GlobalState::inventory.AddCurrentMapKey();}

SecretTreasure::SecretTreasure(EntityPreset*p,Player*):BaseTreasure(p->Position,Drawing::DrawOrder::FG_SPRITES){_dialogueID=-1;}
void SecretTreasure::GetTreasure(){BaseTreasure::GetTreasure();}

// ===========================================================================
// WaterAnim
// ===========================================================================
WaterAnim::WaterAnim(EntityPreset*p,Player*):Entity(p->Position,Drawing::DrawOrder::ENTITIES){exists=false;}
void WaterAnim::Update(){PostUpdate();}

} // namespace AnodyneSharp::Entities

