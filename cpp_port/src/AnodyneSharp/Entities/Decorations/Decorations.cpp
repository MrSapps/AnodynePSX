// Decoration entity implementations
#include "AnodyneSharp/Entities/Decorations/Decorations.hpp"
#include "AnodyneSharp/Entities/EventsAndLights.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Dialogue/DialogueManager.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Utilities/MathUtilities.hpp"

namespace AnodyneSharp::Entities {

// ---- BigTree ----
BigTree::BigTree(EntityPreset* p, Player* /*player*/)
    : Entity(p->Position, "TREE", 64, 64, Drawing::DrawOrder::ENTITIES)
{
    offset = Vector2{16.f, 32.f};
    width  = 32;
    height = 32;
    Position += offset;
    immovable = true;
}
void BigTree::Update()          { Entity::Update(); PostUpdate(); }
void BigTree::Collided(Entity* other) { Separate(this, other); }

// ---- Sign ----
Sign::Sign(EntityPreset* p, Player* /*player*/)
    : Entity(p->Position, "note_rock", 16, 16, Drawing::DrawOrder::ENTITIES)
{
    immovable = true;
    SetFrame(p->Frame_);
}
void Sign::Update()               { PostUpdate(); }
void Sign::Collided(Entity* other){ Separate(this, other); }
bool Sign::PlayerInteraction(Facing /*dir*/) {
    // Frame_ 2 = first sign frame; dialogue index starts at 0
    auto text = Dialogue::DialogueManager::GetDialogue("misc", "any", "solidsprite", Frame() - 2);
    Registry::GlobalState::SetDialogue(text);
    Registry::GlobalState::SetDialogueMode = true;
    return true;
}

// ---- RedCaveEntrance ----
RedCaveEntrance::RedCaveEntrance(EntityPreset* p, int required_events)
    : Entity(p->Position, "red_cave_left", 64, 64, Drawing::DrawOrder::ENTITIES)
{
    offset = Vector2{4.f, 32.f};
    Position += offset;
    width  = 56;
    height = 28;
    immovable = true;

    if (Registry::GlobalState::events.GetEvent(p->TypeValue) < required_events) {
        exists = false;
        auto* toggle = new DoorToggle(Position, (int)width, (int)height);
        Registry::GlobalState::SpawnEntity(toggle);
    }
}
void RedCaveEntrance::Update()               { PostUpdate(); }
void RedCaveEntrance::Collided(Entity* other){ Separate(this, other); }

// ---- DeathFadeIn ----
DeathFadeIn::DeathFadeIn(Color color)
    : Entity({0.f,0.f}, Drawing::DrawOrder::DEATH_FADEIN)
{
    SetTexture("deathFadeIn", 160, 160);
    width  = 160;
    height = 160;
    _color = color;
    opacity = 0.f;
}
void DeathFadeIn::Update() {
    if (opacity < 1.f)
        MathUtilities::MoveTo(opacity, 1.f, 0.9f);
    else
        _done = true;
    PostUpdate();
}
void DeathFadeIn::Draw() { DrawImpl(); }

// ---- Eye_Light ----
Eye_Light::Eye_Light(EntityPreset* p, Player* /*player*/)
    : Entity(p->Position, "eyelight", 16, 16, Drawing::DrawOrder::ENTITIES)
{}
void Eye_Light::Update() { Entity::Update(); PostUpdate(); }

// ---- NonSolid ----
NonSolid::NonSolid(EntityPreset* p, Player* /*player*/)
    : Entity(p->Position, Drawing::DrawOrder::BG_ENTITIES)
{
    // type selects texture
    if (p->TypeValue == "Rail_CROWD")
        SetTexture("rail_CROWD", 16, 22);
    else
        SetTexture("rail", 16, 16);
    // not solid — no collision registration needed
}
void NonSolid::Update() { PostUpdate(); }

// ---- PlayerDieDummy ----
PlayerDieDummy::PlayerDieDummy(Vector2 pos)
    : Entity(pos, Drawing::DrawOrder::PLAYER_DIE_DUMMY)
{
    SetTexture("young_player", 16, 16);
    width  = 10;
    height = 12;
}
void PlayerDieDummy::Update() { Entity::Update(); PostUpdate(); }
void PlayerDieDummy::Draw()   { DrawImpl(); }

} // namespace AnodyneSharp::Entities
