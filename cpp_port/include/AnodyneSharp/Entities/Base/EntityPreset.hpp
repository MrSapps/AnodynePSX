#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"

namespace AnodyneSharp::Entities {

enum class Permanence { GRID_LOCAL, MAP_LOCAL, GLOBAL };

struct EntityState {
    bool Alive     = true;
    bool Activated = false;
};

class EntityPreset {
public:
    std::string TypeName;
    Vector2     Position;
    Guid        EntityID;
    int         Frame_       = 0;
    Permanence  Permanence_  = Permanence::GRID_LOCAL;
    std::string TypeValue;
    int         LinkID       = -1;

    // C++ factory — maps TypeName to a factory function registered at startup
    using FactoryFn = std::function<std::unique_ptr<Entity>(EntityPreset*, class Player*)>;
    static std::unordered_map<std::string, FactoryFn>& Factories();

    EntityPreset(const std::string& typeName, Vector2 pos, Guid entityID,
                 int frame = 0, Permanence perm = Permanence::GRID_LOCAL,
                 const std::string& type = "", int linkid = -1)
        : TypeName(typeName), Position(pos), EntityID(entityID),
          Frame_(frame), Permanence_(perm), TypeValue(type), LinkID(linkid) {}

    std::unique_ptr<Entity> Create(class Player* p) const;

    bool GetAlive() const;
    void SetAlive(bool v);
    bool GetActivated() const;
    void SetActivated(bool v);
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::EntityPreset;
using AnodyneSharp::Entities::Permanence;
using AnodyneSharp::Entities::EntityState;
