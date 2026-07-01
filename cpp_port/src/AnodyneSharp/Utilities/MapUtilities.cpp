#include "AnodyneSharp/Utilities/MapUtilities.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"

namespace AnodyneSharp {

Point MapUtilities::GetRoomCoordinate(const Vector2& pos) {
    int x = (int)(pos.X / GameConstants::SCREEN_WIDTH_IN_PIXELS);
    int y = (int)(pos.Y / GameConstants::SCREEN_HEIGHT_IN_PIXELS);
    return {x, y};
}

Vector2 MapUtilities::GetRoomUpperLeftPos(const Point& pos) {
    return Vector2{
        (float)(pos.X * GameConstants::SCREEN_WIDTH_IN_PIXELS),
        (float)(pos.Y * GameConstants::SCREEN_HEIGHT_IN_PIXELS)
    };
}

Vector2 MapUtilities::GetInGridPosition(const Vector2& pos) {
    return Vector2{
        std::fmod(pos.X, (float)GameConstants::SCREEN_WIDTH_IN_PIXELS),
        std::fmod(pos.Y, (float)GameConstants::SCREEN_HEIGHT_IN_PIXELS)
    };
}

int MapUtilities::GetMapID(const std::string& mapName) {
    static const std::unordered_map<std::string, int> ids = {
        {"STREET",0}, {"OVERWORLD",1}, {"REDCAVE",2}, {"CROWD",3},
        {"APARTMENT",4}, {"HOTEL",5}, {"CIRCUS",6}, {"CLIFF",7},
        {"FOREST",8}, {"WINDMILL",9}, {"REDSEA",10}, {"BEACH",11},
        {"BEDROOM",12}, {"FIELDS",13}, {"GO",14}, {"TERMINAL",15},
        {"HAPPY",16}, {"SPACE",17}, {"CELL",18}, {"SUBURB",19},
        {"BLUE",20}, {"NEXUS",21}, {"BLANK",22}, {"DRAWER",23}, {"DEBUG",24}
    };
    auto it = ids.find(mapName);
    return it != ids.end() ? it->second : -1;
}

} // namespace AnodyneSharp
