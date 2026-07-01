// Interactive entity implementations
#include "AnodyneSharp/Entities/Interactive/AllInteractive.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Dialogue/DialogueManager.hpp"
#include "AnodyneSharp/Utilities/MathUtilities.hpp"

namespace AnodyneSharp::Entities {

// ===========================================================================
// HealthPickup
// ===========================================================================
namespace Interactive {
HealthPickup::HealthPickup(Vector2 pos, bool big)
    : Entity(pos, big ? "big_health_pickup" : "small_health_pickup",
             big ? 16 : 10, big ? 16 : 16, Drawing::DrawOrder::ENTITIES)
    , _healFactor(big ? 3 : 1)
{
    SetSolid(false);
    Play("float");
    exists = false;
}
void HealthPickup::Update() {
    Entity::Update();
    _latency -= GameTimes::DeltaTime();
    PostUpdate();
}
void HealthPickup::Collided(Entity* o) {
    if (_latency <= 0 && dynamic_cast<Player*>(o)) {
        Sounds::SoundManager::PlaySoundEffect("get_small_health");
        GlobalState::CUR_HEALTH = std::min(GlobalState::CUR_HEALTH + _healFactor, GlobalState::MAX_HEALTH_get());
        exists = false;
    }
}
} // namespace Interactive

// ===========================================================================
// DungeonStatue
// ===========================================================================
DungeonStatue::DungeonStatue(EntityPreset* p, Player* /*pl*/)
    : Entity(p->Position, "dungeon_statue", 16, 16, Drawing::DrawOrder::ENTITIES)
    , _preset(p)
{ immovable = true; }
bool DungeonStatue::PlayerInteraction(Facing /*d*/) {
    GlobalState::SetDialogue(Dialogue::DialogueManager::GetDialogue("misc","any","dungeon_statue",0));
    return true;
}

// ===========================================================================
// Elevator
// ===========================================================================
Elevator::Elevator(EntityPreset* p, Player* /*pl*/)
    : Entity(p->Position, "elevator", 16, 16, Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void Elevator::Update() { PostUpdate(); }

// ===========================================================================
// HealthCicada — max-health upgrade; simple enum state machine
//   WAIT_BOSS  : invisible until the dungeon boss is defeated
//   FLYING     : flies to its sentinel position (takes ~1s)
//   ACTIVE     : player can walk into it
//   DONE       : consumed
// ===========================================================================
HealthCicada::HealthCicada(EntityPreset* p, Player* pl)
    : Entity(p->Position, "life_cicada", 16, 16, Drawing::DrawOrder::ENTITIES)
    , _preset(p), _pl(pl)
{
    SetSolid(false);
    _targetPos = p->Position;
    Position.X = -16.f; Position.Y = 30.f;
    visible = false;
    SetFrame(p->Frame_ + 2);
}
void HealthCicada::Update() {
    Entity::Update();
    switch (_state) {
    case State::WAIT_BOSS:
        if (!GlobalState::IsDungeon() || GlobalState::events.BossDefeated.count(GlobalState::CURRENT_MAP_NAME)) {
            _state = State::FLYING;
            visible = true;
            Sounds::SoundManager::PlaySoundEffect("cicada_chirp");
        }
        break;
    case State::FLYING: {
        constexpr float spd = 30.f;
        bool dx = MathUtilities::MoveTo(Position.X, _targetPos.X, spd);
        bool dy = MathUtilities::MoveTo(Position.Y, _targetPos.Y + 20.f, spd);
        if (dx && dy) _state = State::ACTIVE;
        break;
    }
    case State::ACTIVE:
        break;
    case State::DONE:
        exists = false;
        break;
    }
    PostUpdate();
}
void HealthCicada::Collided(Entity* o) {
    if (_state == State::ACTIVE && dynamic_cast<Player*>(o)) {
        GlobalState::MAX_HEALTH_set(GlobalState::MAX_HEALTH_get() + 4);
        GlobalState::CUR_HEALTH = std::min(GlobalState::CUR_HEALTH + 4, GlobalState::MAX_HEALTH_get());
        Sounds::SoundManager::PlaySoundEffect("cicada_chirp");
        _preset->SetAlive(false);
        _state = State::DONE;
    }
}

HealthCicadaSentinel::HealthCicadaSentinel(EntityPreset* p, Player* /*pl*/)
    : Entity(p->Position, Drawing::DrawOrder::ENTITIES)
{ visible = false; }

// ===========================================================================
// Red_Pillar
// ===========================================================================
Red_Pillar::Red_Pillar(EntityPreset* p, Player* /*pl*/)
    : Entity(p->Position, "red_pillar", 16, 32, Drawing::DrawOrder::ENTITIES)
{ immovable = true; }
void Red_Pillar::Update() { PostUpdate(); }

// ===========================================================================
// Rock — reads dialogue from DialogueManager
// ===========================================================================
Rock::Rock(EntityPreset* p, Player* /*pl*/)
    : Entity(p->Position, "note_rock", 16, 16, Drawing::DrawOrder::ENTITIES)
    , _scene(std::to_string(p->Frame_ + 1))
{ immovable = true; }
void Rock::Update() { PostUpdate(); }
void Rock::Collided(Entity* o) { Separate(o, this); }
bool Rock::PlayerInteraction(Facing /*d*/) {
    std::string text = Dialogue::DialogueManager::GetDialogue("rock", "any", _scene, 0);
    if (GlobalState::events.GetEvent("RockTalk") == 0) {
        text = Dialogue::DialogueManager::GetDialogue("misc","any","rock",0) + "^\n" + text;
        GlobalState::events.IncEvent("RockTalk");
    }
    GlobalState::SetDialogue(text);
    return true;
}

// ===========================================================================
// Big_Key — orbit-fly animation then grant big key; state machine version
//   IDLE       : waiting for PlayerInteraction
//   ANIMATING  : orbiting the player and flying inward (simplified)
//   DONE       : awarded, despawn
// ===========================================================================
Big_Key::Big_Key(EntityPreset* p, Player* pl)
    : Entity(p->Position, "key_green", 16, 16, Drawing::DrawOrder::ENTITIES)
    , _preset(p), _pl(pl)
{
    SetFrame(p->Frame_);
    immovable = true;
    width = 9;
    offset.X = 4.f;
    Position.X += 4.f;
}
void Big_Key::Update() {
    Entity::Update();
    if (_state == State::ANIMATING) {
        _timer += GameTimes::DeltaTime();
        _orbitAngle += 3.6f * GameTimes::DeltaTime();
        _orbitRadius = std::max(_orbitRadius - 12.f * GameTimes::DeltaTime(), 0.f);
        Position.X = _pl->Center().X + std::cos(_orbitAngle) * _orbitRadius;
        Position.Y = _pl->Center().Y + std::sin(_orbitAngle) * _orbitRadius;
        if (_timer > 2.f) {
            int idx = _preset->Frame_;
            if (idx >= 0 && idx < 3) GlobalState::inventory.BigKeyStatus[idx] = true;
            Sounds::SoundManager::PlaySoundEffect("gettreasure");
            _preset->SetAlive(false);
            _state = State::DONE;
            exists = false;
        }
    }
    PostUpdate();
}
bool Big_Key::PlayerInteraction(Facing /*d*/) {
    if (_state == State::IDLE) {
        _state = State::ANIMATING;
        _orbitRadius = (_pl->Center() - Center()).Length();
        _orbitAngle = 0.f;
        _timer = 0.f;
    }
    return true;
}

// ===========================================================================
// NPC implementations — constructor + PlayerInteraction (dialogue) + Update
// ===========================================================================

// Macro for NPCs with no dialogue (visual-only presence)
#define NPC_SILENT(ClassName, texture, w, h) \
ClassName::ClassName(EntityPreset*p,Player*pl):Entity(p->Position,texture,w,h,Drawing::DrawOrder::ENTITIES){immovable=true;} \
bool ClassName::PlayerInteraction(Facing){return false;} \
void ClassName::Update(){PostUpdate();}

// Macro for NPCs with simple fixed dialogue
#define NPC_DIAG(ClassName, texture, w, h, npc, scene) \
ClassName::ClassName(EntityPreset*p,Player*pl):Entity(p->Position,texture,w,h,Drawing::DrawOrder::ENTITIES){immovable=true;} \
bool ClassName::PlayerInteraction(Facing){ \
    Registry::GlobalState::SetDialogue(Dialogue::DialogueManager::GetDialogue(npc,scene)); \
    Registry::GlobalState::SetDialogueMode=true; return true;} \
void ClassName::Update(){PostUpdate();}

// Macro for NPCs with dialogue that uses a specific area key (3-arg GetDialogue)
#define NPC_DIAG3(ClassName, texture, w, h, npc, area, scene) \
ClassName::ClassName(EntityPreset*p,Player*pl):Entity(p->Position,texture,w,h,Drawing::DrawOrder::ENTITIES){immovable=true;} \
bool ClassName::PlayerInteraction(Facing){ \
    Registry::GlobalState::SetDialogue(Dialogue::DialogueManager::GetDialogue(npc,area,scene)); \
    Registry::GlobalState::SetDialogueMode=true; return true;} \
void ClassName::Update(){PostUpdate();}

// Silent NPCs (no dialogue — physical/visual presence only)
NPC_SILENT(Fisherman,       "fisherman",        16, 16)
NPC_SILENT(Dam,             "dam",              16, 16)
NPC_SILENT(CellBody,        "cell_body",        16, 16)
NPC_SILENT(CircusFolksDead, "circus_dead",      16, 16)
NPC_SILENT(CliffDog,        "dog",              16, 16)  // has directional dialogue in C# but simplified here
NPC_SILENT(HappyBriar,      "briar",            16, 16)
NPC_SILENT(ForestBriar,     "briar",            16, 16)
NPC_SILENT(BeachBriar,      "briar",            16, 16)
NPC_SILENT(ShadowBriar,     "briar",            16, 16)
NPC_SILENT(HugeFuckingStag, "stag",             32, 32)
NPC_SILENT(Mushroom,        "forest_npcs",      16, 16)
NPC_SILENT(SkittishSecret,  "forest_npcs",      16, 16)
NPC_SILENT(RedWalker,       "redwalker",        32, 48)

// NPCs with simple 2-arg dialogue
NPC_DIAG(HotelGuy,        "hotel_guy",        16, 16, "generic_npc",    "one")
NPC_DIAG(Statue,          "statue",           16, 16, "statue",         "one")
NPC_DIAG(MiaoXiao,        "miao_xiao",        16, 16, "miao",           "philosophy")
NPC_DIAG(Icky,            "icky",             16, 16, "miao",           "icky")
NPC_DIAG(Bunny,           "forest_npcs",      16, 16, "forest_npc",     "bunny")
NPC_DIAG(Thorax,          "forest_npcs",      16, 16, "forest_npc",     "thorax")
NPC_DIAG(Olive,           "fields_npcs",      16, 16, "generic_npc",    "easter_egg")
NPC_DIAG(OutsideMonster,  "monster",          16, 16, "goldman",        "outside")
NPC_DIAG(HairDude,        "redsea_npcs",      16, 16, "generic_npc",    "second")
NPC_DIAG(BombDude,        "redsea_npcs",      16, 16, "generic_npc",    "bomb")
NPC_DIAG(SuburbWalker,    "suburb_walkers",   16, 16, "generic_npc",    "one")

// Quest NPCs — all use same event-based scene
NPC_DIAG(BeachQuest,  "beach_npcs",    16, 16, "generic_npc", "quest_event")
NPC_DIAG(CellQuest,   "cell_quest",    16, 16, "generic_npc", "quest_event")
NPC_DIAG(CliffQuest,  "cliffs_npcs",   16, 16, "generic_npc", "quest_event")
NPC_DIAG(FieldsQuest, "fields_npcs",   16, 16, "generic_npc", "quest_event")
NPC_DIAG(ForestQuest, "forest_npcs",   16, 16, "generic_npc", "quest_event")
NPC_DIAG(GoQuest,     "mitra",         16, 16, "generic_npc", "quest_event")
NPC_DIAG(SpaceQuest,  "space_npcs",    16, 16, "generic_npc", "quest_event")
NPC_DIAG(SuburbQuest, "suburb_walkers",16, 16, "generic_npc", "quest_event")

// NPCs with 3-arg dialogue (npc, area, scene)
NPC_DIAG3(BlankConsole,   "console",          16, 16, "rock",    "NEXUS",      "five")
NPC_DIAG3(InsideMonster,  "monster",          32, 32, "goldman", "etc",        "etc")
NPC_DIAG3(ArthurDanger,   "arthur_javiera",   16, 32, "arthur",  "alone",      "alone")
NPC_DIAG3(JavieraDanger,  "arthur_javiera",   16, 32, "javiera", "alone",      "alone")

// More complex NPCs — simplified to single dialogue call
NPC_DIAG(Snowman,     "snowman",       16, 16, "generic_npc",   "one")
NPC_DIAG(SpaceNPC,    "space_npcs",    16, 16, "geoms",         "one")
NPC_DIAG(HappyNPC,    "happy_npc",     16, 16, "happy_npc",     "one")
NPC_DIAG(CubeKing,    "space_npcs",    32, 32, "cube_king",     "one")
NPC_DIAG(SuburbBlocker,"suburb_walkers",16,16, "suburb_blocker","one")
NPC_DIAG(SuburbIndoors,"suburb_walkers",16,16, "suburb_walker", "one")
NPC_DIAG(WindmillConsole,"console",    16, 16, "generic_npc",   "one")
NPC_DIAG(WindmillShell,"windmill_shell",16,16, "generic_npc",   "one")
NPC_DIAG(Big_Door,    "big_door",      16, 16, "generic_npc",   "one")
NPC_DIAG(Black_Thing, "black_thing",   16, 16, "generic_npc",   "one")
NPC_DIAG(TestNPC,     "young_player",  16, 16, "generic_npc",   "one")
NPC_DIAG(ApartmentEaster,"apartment_easter",16,16,"generic_npc","easter_egg")
NPC_DIAG(RedCaveEaster,  "redcave_easter",  16,16,"generic_npc","easter_egg")
NPC_DIAG(FieldsEaster,   "fields_easter",   16,16,"generic_npc","easter_egg")
NPC_DIAG(DevEaster,      "dev_npcs",        16,16,"generic_npc","easter_egg")
NPC_DIAG(BioFilm,     "biofilm",       16, 16, "generic_npc",   "one")
NPC_DIAG(DeathPlace,  "death_place",   16, 16, "generic_npc",   "one")
NPC_DIAG(EyebossPreview,"eye_boss",    16, 16, "generic_npc",   "one")
NPC_DIAG(HappyEventTrigger,"happy_npc",16,16, "generic_npc",   "one")
NPC_DIAG(PostBlue,    "mitra",         16, 16, "sage",          "one")
NPC_DIAG(EndingSage,  "sage",          16, 16, "briar",         "final")

// Sage — special: different scene per instance, handled in BlueBriar/PostBlue etc.
NPC_DIAG(Sage,        "sage",          16, 16, "sage",          "one")

// Mitra — complex cutscene NPC, simplified to single dialogue
NPC_DIAG3(Mitra,      "mitra",         16, 16, "misc",  "any",  "mitra")

// BlueBriar — plays mitra dialogue after blue boss
NPC_DIAG(BlueBriar,   "briar",         16, 16, "mitra",         "one")

// ShopKeep — simplified to single trade NPC line
NPC_DIAG3(ShopKeep,   "shopkeep",      16, 16, "misc",  "any",  "tradenpc")

// Sadbro — conditional dialogue based on bedroom quest progress; simplified
NPC_DIAG(Sadbro,      "sadbro",        16, 16, "sadbro",        "initial_forced")

#undef NPC_SILENT
#undef NPC_DIAG
#undef NPC_DIAG3

} // namespace AnodyneSharp::Entities
