#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/EntityPreset.hpp"
#include "AnodyneSharp/Entities/Base/EntityPool.hpp"

namespace AnodyneSharp {
namespace Entities { class Player; }

namespace Entities {

class EntityManager {
public:
    static std::unordered_map<Guid, EntityState, Guid::Hash> State;

    static void Initialize();
    static void SetAlive(const Guid& id, bool isAlive);
    static void SetActive(const Guid& id, bool isActive);
    static std::vector<EntityPreset*> GetMapEntities(const std::string& mapName);
    static std::vector<EntityPreset*> GetGridEntities(const std::string& mapName, const Point& grid);
    static struct DoorMapPair* GetLinkedDoor(EntityPreset* door);
    static std::vector<EntityPreset*> GetLinkGroup(int linkid);
    static DoorMapPair* GetNexusGateForCurrentMap();

private:
    static std::unordered_map<std::string, std::vector<std::unique_ptr<EntityPreset>>> _entities;
    static std::unordered_map<int, class DoorPair*> _doorPairs;
    static std::unordered_map<int, std::vector<EntityPreset*>> _linkGroups;

    static void ReadEntities(const std::string& xml);
};

struct DoorMapPair {
    EntityPreset* Door;
    std::string   Map;
    DoorMapPair(EntityPreset* door, const std::string& map) : Door(door), Map(map) {}
};

class DoorPair {
public:
    DoorMapPair pair1;
    DoorMapPair pair2;
    DoorPair(DoorMapPair a, DoorMapPair b) : pair1(a), pair2(b) {}
    DoorMapPair* GetLinkedDoor(EntityPreset* door) {
        if (pair1.Door == door) return &pair2;
        if (pair2.Door == door) return &pair1;
        return nullptr;
    }
};

} // namespace Entities
} // namespace AnodyneSharp

using AnodyneSharp::Entities::EntityManager;
using AnodyneSharp::Entities::DoorMapPair;
using AnodyneSharp::Entities::DoorPair;
