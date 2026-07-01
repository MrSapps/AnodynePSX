#pragma once
#include "AnodyneSharp/Common.hpp"

// Cheat attribute equivalent — marks a method as a cheat command
// In C# this was [CheatAttribute(name, description)]
// In C++ we register cheats manually in CheatzManager

namespace AnodyneSharp::Cheatz {

// Descriptor for a single cheat
struct CheatInfo {
    std::string name;
    std::string description;
    std::function<void(const std::vector<std::string>& args)> execute;
};

} // namespace AnodyneSharp::Cheatz

// Macro to simplify cheat registration (used in CheatzManager.cpp)
#define REGISTER_CHEAT(name, description, fn) \
    RegisterCheat({name, description, fn})
