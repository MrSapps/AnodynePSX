#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"

namespace AnodyneSharp::Entities {

template<typename T>
class EntityPool {
    static_assert(std::is_base_of_v<Entity, T>, "T must derive from Entity");
public:
    std::vector<std::unique_ptr<T>> _pool;

    // View for collision registration
    std::vector<Entity*> Entities() const {
        std::vector<Entity*> result;
        result.reserve(_pool.size());
        for (auto& e : _pool) result.push_back(e.get());
        return result;
    }

    int Alive() const {
        int count = 0;
        for (auto& e : _pool) if (e->exists) ++count;
        return count;
    }

    EntityPool(int total, std::function<std::unique_ptr<T>()> create) {
        _pool.reserve(total);
        for (int i = 0; i < total; ++i) {
            auto e = create();
            e->exists = false;
            _pool.push_back(std::move(e));
        }
    }

    bool Spawn(std::function<void(T*)> onSpawn = nullptr, int total = 1, bool force = false) {
        bool result = false;
        int spawned = 0;
        for (auto& e : _pool) {
            if (spawned >= total) break;
            if (!e->exists || force) {
                e->exists = true;
                if (onSpawn) onSpawn(e.get());
                result = true;
                ++spawned;
            }
        }
        return result;
    }
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::EntityPool;
