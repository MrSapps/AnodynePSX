#pragma once
#include "AnodyneSharp/Common.hpp"
#include "XNA/Graphics.hpp"

namespace AnodyneSharp::Drawing {

enum class DrawOrder {
    BACKGROUND = 0,
    MAP_BG,
    PLAYER_REFLECTION,
    FOOT_OVERLAY,
    MAP_BG2,
    VERY_BG_ENTITIES,
    BG_ENTITIES,
    SHADOWS,
    PARTICLES,
    ROLLERS,
    ENTITIES,
    FG_SPRITES,
    MAP_FG,
    DEC_OVER,
    DARKNESS,
    HITBOX,
    CREDITS_OVERLAY,
    HEADER,
    UI_OBJECTS,
    EQUIPPED_BORDER,
    DUST_ICON,
    HEALTH_UPGRADE,
    PAUSE_BG,
    EQUIPMENT_ICON,
    EQUIPPED_ICON,
    AUDIO_SLIDER,
    PAUSE_SELECTOR,
    MENUTEXT,
    MINIMAP,
    MINIMAP_PLAYER,
    MINIMAP_CHEST,
    TEXTBOX,
    TEXT,
    SUBMENU_SLIDER,
    SUBMENU_SELECTOR,
    DEATH_FADEIN,
    PLAYER_DIE_DUMMY,
    DEATH_TEXT,
    BLACK_OVERLAY
};

class DrawingUtilities {
public:
    static float GetDrawingZ(DrawOrder order, float gridy);
    static float GetDrawingZ(DrawOrder order);
};

} // namespace AnodyneSharp::Drawing

using AnodyneSharp::Drawing::DrawOrder;
using AnodyneSharp::Drawing::DrawingUtilities;
