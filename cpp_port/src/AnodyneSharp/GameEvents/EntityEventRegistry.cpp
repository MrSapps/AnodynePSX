#include "AnodyneSharp/GameEvents/EntityEventRegistry.hpp"

namespace AnodyneSharp::GameEvents {

std::unordered_map<uint64_t, EntityEventRegistry::Handler> EntityEventRegistry::_handlers;

void EntityEventRegistry::Register(const Guid& id, Handler handler) {
    _handlers[id.Lo ^ (id.Hi << 1)] = std::move(handler);
}

void EntityEventRegistry::Unregister(const Guid& id) {
    _handlers.erase(id.Lo ^ (id.Hi << 1));
}

void EntityEventRegistry::Dispatch(const GameEvent& ev) {
    for (auto& [key, handler] : _handlers) {
        handler(ev);
    }
}

void EntityEventRegistry::Clear() { _handlers.clear(); }

} // namespace AnodyneSharp::GameEvents
