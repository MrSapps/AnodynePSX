#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp {

// Interface for IMod
class IMod {
public:
    virtual ~IMod() = default;
    virtual void Update() {}
    virtual void Initialize() {}
};

// ModLoader
class ModLoader {
public:
    static std::vector<std::unique_ptr<IMod>> mods;
    static void Initialize();
};

// Cheatz
namespace Cheatz {

class CheatzManager {
public:
    static bool IsCheatEnabled(const std::string& cheatName);
    static void ToggleFuckItMode();
    static void Update();
};

} // namespace Cheatz
} // namespace AnodyneSharp

using AnodyneSharp::IMod;
using AnodyneSharp::ModLoader;
