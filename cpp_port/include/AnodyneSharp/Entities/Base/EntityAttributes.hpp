#pragma once
#include "AnodyneSharp/Common.hpp"

// C++ port of C# [Collision] attribute
// Stored as per-class static data

namespace AnodyneSharp::Entities {

struct CollisionAttribute {
    std::vector<std::string> TypeNames; // names of target types to collide with
    bool MapCollision  = false;
    bool PartOfMap     = false;
    bool KeepOnScreen  = false;
};

// C++ port of C# [AlwaysSpawn] attribute
struct AlwaysSpawnAttribute {};

// C++ port of C# [Enemy] attribute  
struct EnemyAttribute {};

} // namespace AnodyneSharp::Entities
