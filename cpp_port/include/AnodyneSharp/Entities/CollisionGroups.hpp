#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/EntityAttributes.hpp"
#include <typeindex>

namespace AnodyneSharp {
namespace MapData { class Map; }

namespace Entities {

class CollisionGroups {
public:
    CollisionGroups(int num_enemies_killed);

    int  KilledEnemies() const;
    void Register(Entity* e);
    void DoCollision(MapData::Map* map, bool ignore_player_map_collision);

private:
    struct Group {
        std::vector<Entity*> targets;
        std::vector<Entity*> colliders;
    };

    std::unordered_map<std::type_index, Group> _groups;
    std::vector<Entity*> _mapColliders;
    std::vector<Entity*> _mapEntities;
    std::vector<Entity*> _enemies;
    std::vector<Entity*> _keepOnScreen;
    int base_killed_enemies;

    Group& Get(std::type_index t);
};

} // namespace Entities
} // namespace AnodyneSharp

using AnodyneSharp::Entities::CollisionGroups;
