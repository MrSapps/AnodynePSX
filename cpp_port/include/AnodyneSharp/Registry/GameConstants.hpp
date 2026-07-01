#pragma once
#include "AnodyneSharp/Common.hpp"

// Forward declaration
namespace AnodyneSharp::Dialogue { enum class Language; }

namespace AnodyneSharp::Registry {

class GlobalState; // forward

class GameConstants {
public:
    enum class MapOrder {
        STREET, OVERWORLD, REDCAVE, CROWD, APARTMENT, HOTEL,
        CIRCUS, CLIFF, FOREST, WINDMILL, REDSEA, BEACH,
        BEDROOM, FIELDS, GO, TERMINAL, HAPPY, SPACE, CELL,
        SUBURB, BLUE, NEXUS, BLANK, DRAWER, MAP_DEBUG
    };

    static std::string SavePath;

    static constexpr int SCREEN_WIDTH_IN_TILES  = 10;
    static constexpr int SCREEN_HEIGHT_IN_TILES = 10;
    static constexpr int TILE_WIDTH             = 16;
    static constexpr int TILE_HEIGHT            = 16;
    static constexpr int SCREEN_WIDTH_IN_PIXELS  = 160;
    static constexpr int SCREEN_HEIGHT_IN_PIXELS = 160;
    static constexpr int HEADER_HEIGHT           = 20;
    static constexpr int BUTTON_WIDTH            = 13;
    static constexpr int BUTTON_HEIGHT           = 14;

    static int FONT_LINE_HEIGHT();
    static int LineOffset();

    static void Init();
};

} // namespace AnodyneSharp::Registry

using AnodyneSharp::Registry::GameConstants;
