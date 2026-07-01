#pragma once
#include "AnodyneSharp/Common.hpp"
#include "AnodyneSharp/GameEvents/Events.hpp"

namespace AnodyneSharp::GameEvents {

// C# EntityEventRegistry — maps entity GUIDs to event handlers
// Entity::OnEvent is called when a matching event fires
class EntityEventRegistry {
public:
    using Handler = std::function<void(const GameEvent&)>;

    static void Register(const Guid& entityId, Handler handler);
    static void Unregister(const Guid& entityId);
    static void Dispatch(const GameEvent& ev);
    static void Clear();

private:
    static std::unordered_map<uint64_t, Handler> _handlers;
};

} // namespace AnodyneSharp::GameEvents
