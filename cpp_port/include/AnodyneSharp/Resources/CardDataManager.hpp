#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp::Resources {

// Maps map-name → {grid_key → cardId}.
// grid_key = grid_y * 1000 + grid_x  (room coordinates, not pixels).
class CardDataManager {
public:
    // Parse CardLookup.card text file into _cardLocations.
    static void Load(const std::string& path);

    // Returns the card ID at the player's current map/grid, or -1 if not found.
    static int  GetCardId();

    // True if every "normal" card (id<=36 || id==43) in mapName has been collected.
    static bool GotAllNormalCards(const std::string& mapName);

    // True if GotAllNormalCards() for any map that has normal cards.
    static bool GotAllNormalCardsOfAnyMap();

private:
    static int GridKey(int x, int y) { return y * 1000 + x; }
    // mapName → { GridKey(gridX,gridY) → cardId }
    static std::unordered_map<std::string, std::unordered_map<int, int>> _cardLocations;
};

} // namespace AnodyneSharp::Resources

using AnodyneSharp::Resources::CardDataManager;
