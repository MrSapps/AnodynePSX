#pragma once
#include "AnodyneSharp/Common.hpp"
#include "XNA/Framework.hpp"
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"

// Pause menu UI types

namespace AnodyneSharp::UI::PauseMenu {

class MenuSelector : public Entities::Entity {
public:
    MenuSelector(Drawing::DrawOrder layer = Drawing::DrawOrder::PAUSE_SELECTOR);
    void Update()     override;
    void PostUpdate() override;
    void Draw()       override;
};

// PauseMenuSelector
class PauseMenuSelector : public Entities::Entity {
public:
    PauseMenuSelector();
    void Update()     override;
    void DrawUI();
};

// Equipment display
class Equipment {
public:
    void Draw();
    void Update();
};

// Config options
class UIOption {
public:
    virtual ~UIOption() = default;
    virtual void Update() {}
    virtual void Draw()   {}
    virtual bool IsSelected() const { return false; }
    bool enabled = true;
};

class ActionOption     : public UIOption {};
class AudioSlider      : public UIOption {};
class CheckBox         : public UIOption {};
class OptionSelector   : public UIOption {};
class SubstateOption   : public UIOption {};
class TextSelector     : public UIOption {};

} // namespace AnodyneSharp::UI::PauseMenu

using AnodyneSharp::UI::PauseMenu::MenuSelector;
using AnodyneSharp::UI::PauseMenu::Equipment;
