#pragma once
#include "AnodyneSharp/UI/Font/Font.hpp"
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"

namespace AnodyneSharp::UI {

// UIEntity - a screen-space entity (not world-space)
class UIEntity : public Entities::Entity {
public:
    bool IsVisible = true;
    float opacity  = 1.f;
    Vector2 velocity = {0,0};

    UIEntity(Vector2 pos, const std::string& textureName, int w, int h,
             Drawing::DrawOrder layer);
    UIEntity(Vector2 pos, const std::string& textureName, int w, int h,
             Entities::Base::Rendering::ILayerType* layer);
    UIEntity(Vector2 pos, std::unique_ptr<Entities::Base::Rendering::ISpriteRenderer> renderer,
             Drawing::DrawOrder layer,
             SpriteEffects flip = SpriteEffects::None);

    void Draw() override;
    void PostUpdate() override;
};

// UILabel - text label
class UILabel : public UIEntity {
public:
    bool IsVisible = true;
    TextWriter* Writer = nullptr;

    UILabel(Vector2 pos, bool drawShadow, const std::string& text,
            Color color = Color::White,
            Drawing::DrawOrder layer = Drawing::DrawOrder::TEXT,
            bool centerText = false);

    void Draw() override;
    void SetText(const std::string& text);
    const std::string& GetText() const { return _text; }
private:
    std::string _text;
    std::unique_ptr<Font::SpriteFont> _ownedFont;
    TextWriter                         _ownedWriter;
};

// TextBox
class TextBox {
public:
    bool PauseWriting   = false;
    bool BlinkyEnabled  = true;
    TextWriter Writer;   // owned text writer; set up in ctor

    TextBox(bool useMenuBox = false, bool isIntro = false);
    void Update();
    void DrawUI();

private:
    std::unique_ptr<Font::SpriteFont> _font;
    Vector2 _pos       = {0,0};
    Vector2 _blinkyPos = {0,0};
    float   _blinkyTimer   = 0.4f;
    bool    _blinkyVisible = false;
    bool    _useMenuBox    = false;
};

// DustIcon
class DustIcon : public UIEntity {
public:
    DustIcon();
    void Update() override;
};

// Health bar
class HealthBar {
public:
    void Draw();
    void Update();
};

} // namespace AnodyneSharp::UI

using AnodyneSharp::UI::UIEntity;
using AnodyneSharp::UI::UILabel;
using AnodyneSharp::UI::TextBox;
using AnodyneSharp::UI::HealthBar;
// TextWriter, SpriteFont, FontManager, TextCharacter — re-exported from Font/Font.hpp
