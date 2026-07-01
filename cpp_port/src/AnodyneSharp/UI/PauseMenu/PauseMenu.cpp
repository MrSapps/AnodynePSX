#include "AnodyneSharp/UI/PauseMenu/PauseMenu.hpp"
#include "AnodyneSharp/Entities/Base/Rendering/SpriteRenderer.hpp"
#include "AnodyneSharp/Drawing/Spritesheet/Anim.hpp"
#include "AnodyneSharp/Input/KeyInput.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"

namespace AnodyneSharp::UI::PauseMenu {

// ---- MenuSelector ----
MenuSelector::MenuSelector(Drawing::DrawOrder layer)
    : Entities::Entity(
        {0.f, 0.f},
        std::make_unique<Entities::Base::Rendering::AnimatedSpriteRenderer>("arrows", 7, 7,
            Anim{"disabledRight", {2},    1.f},
            Anim{"disabledLeft",  {0},    1.f},
            Anim{"enabledLeft",   {0, 1}, 6.f},
            Anim{"enabledRight",  {3, 2}, 6.f}),
        layer)
{
    Play("enabledRight");
}
void MenuSelector::Update()     { Entities::Entity::Update(); }
void MenuSelector::PostUpdate() { Entities::Entity::PostUpdate(); }
void MenuSelector::Draw()       { if (visible) DrawImpl(); }

// ---- PauseMenuSelector ----
PauseMenuSelector::PauseMenuSelector()
    : Entities::Entity({0,0}, Drawing::DrawOrder::PAUSE_SELECTOR) {}
void PauseMenuSelector::Update()  {}
void PauseMenuSelector::DrawUI()  {}

// ---- Equipment ----
void Equipment::Draw()   {}
void Equipment::Update() {}

} // namespace AnodyneSharp::UI::PauseMenu
