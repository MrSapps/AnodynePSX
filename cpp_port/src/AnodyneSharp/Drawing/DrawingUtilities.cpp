#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"

namespace AnodyneSharp::Drawing {

float DrawingUtilities::GetDrawingZ(DrawOrder order, float gridy) {
    float z = (float)(int)order;
    if (order == DrawOrder::ENTITIES) {
        z += gridy / (GameConstants::SCREEN_HEIGHT_IN_PIXELS + 1);
    } else if ((int)order > (int)DrawOrder::HITBOX) {
        return GetDrawingZ(order);
    }
    return 1.f - z / (float)(int)DrawOrder::HITBOX;
}

float DrawingUtilities::GetDrawingZ(DrawOrder order) {
    float z = (float)(int)order;
    return 1.f - z / (float)(int)DrawOrder::BLACK_OVERLAY;
}

} // namespace AnodyneSharp::Drawing
