#pragma once
#include "AnodyneSharp/Common.hpp"
#include <typeindex>
#include <vector>

// Mirrors C# [Events(...)] attribute
// In C++ we use a base class approach and registration macros

// EVENTS_ATTRIBUTE macro - marks a class as listening for specific events
// Usage: put DECLARE_EVENTS(Type1, Type2) in class declaration
#define DECLARE_EVENTS(...) \
    static std::vector<std::type_index> GetEventTypes() { \
        return { typeid(__VA_ARGS__) }; \
    }

namespace AnodyneSharp::GameEvents {

// EventsAttribute data (stored per type, not via C++ reflection)
// Entities register their event types manually
struct EventsAttributeData {
    std::vector<std::type_index> Types;
};

} // namespace AnodyneSharp::GameEvents
