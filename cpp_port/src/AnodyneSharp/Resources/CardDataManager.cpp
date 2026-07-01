#include "AnodyneSharp/Resources/CardDataManager.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include <fstream>
#include <sstream>

namespace AnodyneSharp::Resources {

std::unordered_map<std::string, std::unordered_map<int, int>> CardDataManager::_cardLocations;

// File format (CardLookup.card):
//   MAPNAME
//   {
//       id   gridX   gridY
//       ...
//   }
void CardDataManager::Load(const std::string& path) {
    _cardLocations.clear();
    std::ifstream f(path);
    if (!f.is_open()) return;

    std::string line, curMap;
    bool inBlock = false;
    while (std::getline(f, line)) {
        // trim whitespace
        auto b = line.find_first_not_of(" \t\r\n");
        if (b == std::string::npos) continue;
        line = line.substr(b);

        if (line == "{") { inBlock = true; continue; }
        if (line == "}") { inBlock = false; curMap.clear(); continue; }

        if (!inBlock) {
            curMap = line;
            _cardLocations[curMap] = {};
        } else {
            std::istringstream ss(line);
            int id, gx, gy;
            if (ss >> id >> gx >> gy)
                _cardLocations[curMap][GridKey(gx, gy)] = id;
        }
    }
}

int CardDataManager::GetCardId() {
    const std::string& map = Registry::GlobalState::CURRENT_MAP_NAME;
    if (map == "NEXUS") return 48;

    auto mi = _cardLocations.find(map);
    if (mi == _cardLocations.end()) return -1;

    Point grid = Registry::GlobalState::CurrentMapGrid();
    int key = GridKey(grid.X, grid.Y);
    auto ci = mi->second.find(key);
    return ci != mi->second.end() ? ci->second : -1;
}

bool CardDataManager::GotAllNormalCards(const std::string& mapName) {
    auto mi = _cardLocations.find(mapName);
    if (mi == _cardLocations.end()) {
        // No card data — treat as done if nexus portal activated
        return Registry::GlobalState::events.ActivatedNexusPortals.count(mapName) > 0;
    }
    for (auto& [key, id] : mi->second) {
        bool isNormal = (id <= 36 || id == 43);
        if (isNormal && !Registry::GlobalState::inventory.CardStatus[id])
            return false;
    }
    return true;
}

bool CardDataManager::GotAllNormalCardsOfAnyMap() {
    for (auto& [map, cards] : _cardLocations) {
        bool hasNormal = false;
        for (auto& [k, id] : cards)
            if (id <= 36 || id == 43) { hasNormal = true; break; }
        if (hasNormal && GotAllNormalCards(map)) return true;
    }
    return false;
}

} // namespace AnodyneSharp::Resources
