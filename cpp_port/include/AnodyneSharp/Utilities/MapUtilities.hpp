#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp {

class MapUtilities {
public:
    static Point GetRoomCoordinate(const Vector2& pos);
    static Vector2 GetRoomUpperLeftPos(const Point& pos);
    static Vector2 GetInGridPosition(const Vector2& pos);
    static int GetMapID(const std::string& mapName);
};

} // namespace AnodyneSharp

using AnodyneSharp::MapUtilities;
