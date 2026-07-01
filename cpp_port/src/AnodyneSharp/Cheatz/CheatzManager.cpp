#include "AnodyneSharp/Cheatz/CheatzManager.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Registry/InventoryManager.hpp"
#include <sstream>

namespace AnodyneSharp::Cheatz {

std::vector<CheatInfo> CheatzManager::_cheats;

void CheatzManager::Initialize() {
    _cheats.clear();

    // Example cheats (matching original C# cheats)
    REGISTER_CHEAT("maxhealth", "Set health to maximum",
        [](const std::vector<std::string>&) {
            GlobalState::CUR_HEALTH = GlobalState::MAX_HEALTH_get();
        });

    REGISTER_CHEAT("allcards", "Get all cards",
        [](const std::vector<std::string>&) {
            using namespace Registry;
            auto& inv = GlobalState::inventory;
            for (int i = 0; i < (int)inv.CardStatus.size(); ++i)
                inv.CardStatus[i] = true;
        });

    REGISTER_CHEAT("allbrooms", "Unlock all broom types",
        [](const std::vector<std::string>&) {
            // Nothing to unlock — brooms are picked up via items
        });

    REGISTER_CHEAT("tp", "Teleport to map,gx,gy",
        [](const std::vector<std::string>& args) {
            if (args.size() >= 3) {
                GlobalState::CURRENT_MAP_NAME = args[0];
                GlobalState::CURRENT_GRID_X   = std::stoi(args[1]);
                GlobalState::CURRENT_GRID_Y   = std::stoi(args[2]);
            }
        });
}

bool CheatzManager::ExecuteCheat(const std::string& input) {
    std::istringstream ss(input);
    std::string name; ss >> name;
    std::vector<std::string> args;
    std::string arg;
    while (ss >> arg) args.push_back(arg);

    for (auto& c : _cheats) {
        if (c.name == name) {
            c.execute(args);
            return true;
        }
    }
    return false;
}

const std::vector<CheatInfo>& CheatzManager::GetCheats() {
    return _cheats;
}

void CheatzManager::RegisterCheat(CheatInfo info) {
    _cheats.push_back(std::move(info));
}

} // namespace AnodyneSharp::Cheatz
