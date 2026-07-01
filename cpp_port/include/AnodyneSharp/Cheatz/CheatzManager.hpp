#pragma once
#include "AnodyneSharp/Cheatz/CheatAttribute.hpp"

namespace AnodyneSharp::Cheatz {

class CheatzManager {
public:
    static void Initialize();
    static bool ExecuteCheat(const std::string& input);
    static const std::vector<CheatInfo>& GetCheats();

private:
    static void RegisterCheat(CheatInfo info);
    static std::vector<CheatInfo> _cheats;
};

} // namespace AnodyneSharp::Cheatz
