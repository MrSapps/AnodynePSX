#include "AnodyneSharp/UI/UITypes.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include "SDL3/SDL.h"

namespace AnodyneSharp::UI {

// ---- UIEntity ----
UIEntity::UIEntity(Vector2 pos, const std::string& tex, int w, int h, Drawing::DrawOrder layer)
    : Entities::Entity(pos, tex, w, h, layer) {}

UIEntity::UIEntity(Vector2 pos, const std::string& tex, int w, int h,
                   Entities::Base::Rendering::ILayerType* layer)
    : Entities::Entity(pos, tex, w, h, layer) {}

UIEntity::UIEntity(Vector2 pos,
                   std::unique_ptr<Entities::Base::Rendering::ISpriteRenderer> renderer,
                   Drawing::DrawOrder layer, SpriteEffects /*flip*/)
    : Entities::Entity(pos, layer) { (void)renderer; }

void UIEntity::Draw()       { DrawImpl(); }
void UIEntity::PostUpdate() { Entities::Entity::PostUpdate(); }

// ---- UILabel ----
UILabel::UILabel(Vector2 pos, bool drawShadow, const std::string& text,
                 Color color, Drawing::DrawOrder layer, bool centerText)
    : UIEntity(pos, "font-white-apple-7x8", 7, 8, layer), _text(text)
{
    visible = true; // always visible; matches C# UILabel where 2nd arg is drawShadow
    _ownedFont = std::make_unique<Font::SpriteFont>(Font::FontManager::InitFont(color));
    _ownedWriter = TextWriter{(int)pos.X, (int)pos.Y, 200, 200};
    _ownedWriter.SetSpriteFont(_ownedFont.get(), nullptr);
    _ownedWriter.DrawShadow = drawShadow;
    _ownedWriter.CenterText = centerText;
    _ownedWriter.Text = text;
    _ownedWriter.ResetTextProgress(); // re-syncs AtEndOfText now that Text is non-empty
    _ownedWriter.ProgressTextToEnd();
    Writer = &_ownedWriter;
}

void UILabel::Draw() {
    if (!visible) return;
    static int _lc = 0;
    if (++_lc <= 5) {
        SDL_Log("UILabel::Draw text='%s' hasFnt=%d fntTex=%p lines=%d",
            _text.c_str(),
            (int)(_ownedFont != nullptr),
            (void*)(_ownedFont ? _ownedFont->texture : nullptr),
            _ownedWriter.LineCount());
    }
    // Use Writer if set, otherwise fall back to DrawImpl
    if (Writer) Writer->Draw();
    else        DrawImpl();
}
void UILabel::SetText(const std::string& t) {
    _text = t;
    _ownedWriter.Text = t;
    _ownedWriter.ResetTextProgress();
    _ownedWriter.ProgressTextToEnd();
    if (Writer && Writer != &_ownedWriter) Writer->Text = t;
}

// ---- TextBox ----
static constexpr int   TB_WIDTH  = 156;
static constexpr int   TB_HEIGHT = 44;
static constexpr float BLINKY_PERIOD = 0.4f;

TextBox::TextBox(bool useMenuBox, bool isIntro)
    : _useMenuBox(useMenuBox) {
    // Position: bottom of screen unless DialogueTop
    float y = GlobalState::DialogueTop
                ? (float)GameConstants::HEADER_HEIGHT
                : (float)(GameConstants::SCREEN_HEIGHT_IN_PIXELS - TB_HEIGHT - 4);
    _pos = {2.f, y};
    _blinkyPos = {(float)(TB_WIDTH - 8), y + TB_HEIGHT - 10.f};

    _font = std::make_unique<Font::SpriteFont>(Font::FontManager::InitFont(Color::White));
    Writer.SetSpriteFont(_font.get(), nullptr);
    Writer.SetWriteArea((int)_pos.X + 4, (int)_pos.Y + 8, TB_WIDTH - 8, TB_HEIGHT - 14);
}

void TextBox::Update() {
    if (!PauseWriting) Writer.Update();
    if (BlinkyEnabled) {
        _blinkyTimer -= GameTimes::DeltaTime();
        if (_blinkyTimer <= 0.f) {
            _blinkyVisible = !_blinkyVisible;
            _blinkyTimer = BLINKY_PERIOD;
        }
    }
}

void TextBox::DrawUI() {
    using namespace Drawing;
    float z  = DrawingUtilities::GetDrawingZ(DrawOrder::TEXTBOX);
    float z2 = DrawingUtilities::GetDrawingZ(DrawOrder::TEXT);
    auto* boxTex = Resources::ResourceManager::GetTexture(
        _useMenuBox ? "menudialogue_box" : "dialogue_box");
    if (boxTex)
        SpriteDrawer::DrawSprite(boxTex, _pos, nullptr, nullptr, 0.f, 1.f, z);
    Writer.Draw();
    if (BlinkyEnabled && _blinkyVisible && (Writer.AtEndOfText || Writer.AtEndOfBox)) {
        auto* blink = Resources::ResourceManager::GetTexture("dialogue_blinky_box");
        if (blink)
            SpriteDrawer::DrawSprite(blink, _blinkyPos, nullptr, nullptr, 0.f, 1.f, z2);
    }
}

// ---- DustIcon ----
DustIcon::DustIcon() : UIEntity({0,0}, "dust_ui", 8, 8, Drawing::DrawOrder::DUST_ICON) {}
void DustIcon::Update() { UIEntity::PostUpdate(); }

// ---- HealthBar ----
void HealthBar::Update() {}   // health read from GlobalState at Draw time

void HealthBar::Draw() {
    using namespace Drawing;
    auto* tex = Resources::ResourceManager::GetTexture("health_piece");
    if (!tex) return;
    float z = DrawingUtilities::GetDrawingZ(DrawOrder::UI_OBJECTS);
    int cur = GlobalState::CUR_HEALTH;
    int max = GlobalState::MAX_HEALTH_get();
    for (int i = 0; i < max; i++) {
        // Layout mirrors C# HealthBar: pieces grow left then up, 8 per row
        float px = 155.f - 11.f - 8.f * (float)(7 - i % 8) - 7.f * (float)(i / 8);
        float py = 2.f   + (float)(i / 8) * 7.f;
        Rectangle src = { (i < cur) ? 0 : 11, 0, 11, 6 };
        SpriteDrawer::DrawSprite(tex, {px, py}, &src, nullptr, 0.f, 1.f, z);
    }
}

} // namespace AnodyneSharp::UI
