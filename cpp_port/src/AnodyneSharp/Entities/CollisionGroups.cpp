#include "AnodyneSharp/Entities/CollisionGroups.hpp"
#include "AnodyneSharp/Entities/Base/HealthDropper.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/MapData/Map.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Utilities/MapUtilities.hpp"

namespace AnodyneSharp::Entities {

CollisionGroups::CollisionGroups(int num_enemies_killed)
    : base_killed_enemies(num_enemies_killed) {}

int CollisionGroups::KilledEnemies() const {
    int dead = 0;
    for (auto* e : _enemies) if (!e->exists) ++dead;
    return base_killed_enemies + dead;
}

void CollisionGroups::Register(Entity* e) {
    // Register in type hierarchy (simplified - no RTTI traversal like C#)
    _groups[typeid(*e)].targets.push_back(e);

    // Map collision
    _mapColliders.push_back(e);

    // Track enemies (simplified - all entities with health)
    auto* hd = dynamic_cast<HealthDropper*>(e);
    if (hd) _enemies.push_back(e);
}

void CollisionGroups::DoCollision(MapData::Map* map, bool ignore_player_map_collision) {
    for (auto* e : _mapColliders) {
        if (!e->exists) continue;
        Touching saved = e->allowCollisions;
        auto* p = dynamic_cast<Player*>(e);
        if (ignore_player_map_collision && p) e->SetSolid(false);
        map->Collide(e);
        e->allowCollisions = saved;
    }
    for (auto& [type, group] : _groups) {
        for (auto* collider : group.colliders) {
            if (!collider->exists) continue;
            for (auto* target : group.targets) {
                if (!target->exists || target == collider) continue;
                Rectangle ch = collider->Hitbox();
                Rectangle th = target->Hitbox();
                if (ch.X < th.X+th.Width && ch.X+ch.Width > th.X &&
                    ch.Y < th.Y+th.Height && ch.Y+ch.Height > th.Y) {
                    collider->Collided(target);
                }
            }
        }
    }

    // Keep on screen
    Vector2 roomTL = MapUtilities::GetRoomUpperLeftPos(GlobalState::CurrentMapGrid());
    Vector2 roomBR = {roomTL.X + 160.f, roomTL.Y + 160.f};
    for (auto* e : _keepOnScreen) {
        if (!e->exists) continue;
        Rectangle h = e->Hitbox();
        if (h.X < (int)roomTL.X) { e->Position.X = roomTL.X; e->touching |= Touching::LEFT; }
        else if (h.X+h.Width > (int)roomBR.X) { e->Position.X = roomBR.X - e->width; e->touching |= Touching::RIGHT; }
        if (h.Y < (int)roomTL.Y) { e->Position.Y = roomTL.Y; e->touching |= Touching::UP; }
        else if (h.Y+h.Height > (int)roomBR.Y) { e->Position.Y = roomBR.Y - e->height; e->touching |= Touching::DOWN; }
    }
}

CollisionGroups::Group& CollisionGroups::Get(std::type_index t) {
    return _groups[t];
}

} // namespace AnodyneSharp::Entities
