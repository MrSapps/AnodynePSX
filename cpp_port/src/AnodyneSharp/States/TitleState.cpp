#include "AnodyneSharp/States/TitleState.hpp"
#include "AnodyneSharp/States/AllStates.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "AnodyneSharp/Input/KeyInput.hpp"

namespace AnodyneSharp::States {

// ---- TitleState ----
TitleState::TitleState() 
{
    
}

void TitleState::Create() {
    //_background.Load("title_bg", 0.f, -30.f);
    
    
    // nexusImage
//    mNexusImage = std::make_unique<UIEntity>(Vector2(0, 180), "door", GameConstants::SCREEN_WIDTH_IN_PIXELS, 116, DrawOrder::UI_OBJECTS);
    mNexusImage = std::make_unique<UIEntity>(Vector2(0, -180), "door", GameConstants::SCREEN_WIDTH_IN_PIXELS, 116, DrawOrder::UI_OBJECTS);

    mNexusImage->velocity.Y = 20.0f;

    mDoorGlow = std::make_unique<UIEntity>(Vector2(64, 32), "door_glow", 64, 32, new RefLayer(mNexusImage->layer_def_get(), 1)); // TODO: fix leak
    mDoorSpin1 = std::make_unique<UIEntity>(Vector2(0, 0), "door_spinglow1", 64, 64, new RefLayer(mNexusImage->layer_def_get(), 2)); // TODO: fix leak
    mDoorSpin2 = std::make_unique<UIEntity>(Vector2(0, 0), "door_spinglow2", 64, 64, new RefLayer(mNexusImage->layer_def_get(), 2)); // TODO: fix leak
    mPressEnterTex = std::make_unique<UIEntity>(Vector2((GameConstants::SCREEN_WIDTH_IN_PIXELS - 96) / 2, GameConstants::SCREEN_HEIGHT_IN_PIXELS), "press_enter", 96, 16, DrawOrder::MENUTEXT);

    mTitleTex      = std::make_unique<UIEntity>(Vector2(0, 0), "title_text", 128, 48, DrawOrder::MENUTEXT);
    mTitleOverlay      = std::make_unique<UIEntity>(Vector2(0, 0), "title_text_white", 128, 48, DrawOrder::TEXTBOX);

    mSubtitle = std::make_unique<UIEntity>(Vector2(16), "title_remake", 71, 11, DrawOrder::MENUTEXT);
    mSubtitleOverlay      = std::make_unique<UIEntity>(Vector2(0, 0), "title_remake_white", 71, 11, DrawOrder::TEXTBOX);

    // TODO: Not here in CS
    GlobalState::flash.Flash(2.f, Color::Black);

    // TODO ??
    //GlobalState::TitleScreenFinish;
    //GlobalState.TitleScreenFinish.Entities.Add(pressEnter);

    Sounds::SoundManager::PlaySong("title");
}

void TitleState::Update() {
    if (_pixelating) {
        GlobalState::pixelation.AddPixelation(15.f);
        GlobalState::black_overlay.ChangeAlpha(0.54f);
        if (GlobalState::black_overlay.alpha >= 1.f) {
            GlobalState::pixelation.SetPixelation(0.f);
            GlobalState::black_overlay.alpha = 0.f;
            GlobalState::flash.Deactivate();
            GlobalState::GameState->SetState<MainMenuState>();
        }
        return;
    }

    if (!GlobalState::flash.Active()) {
        _blinkTimer -= GameTimes::DeltaTime();
        if (_blinkTimer <= 0.f) {
            _blinkTimer = 1.f;
            _pressEnterVisible = !_pressEnterVisible;
        }
        using namespace Input;
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel) ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Up)     ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Down)) {
            _pixelating = true;
        }
    }

    _background.Update();

    mNexusImage->Update();
    mDoorGlow->Update();
    mDoorSpin1->Update();
    mDoorSpin2->Update();
    mTitleTex->Update();
    mTitleOverlay->Update();
    mPressEnterTex->Update();

    
    mNexusImage->PostUpdate();
    mDoorGlow->PostUpdate();
    mDoorSpin1->PostUpdate();
    mDoorSpin2->PostUpdate();
    mTitleTex->PostUpdate();
    mTitleOverlay->PostUpdate();
    mPressEnterTex->PostUpdate();
    
}

void TitleState::Draw()    {}

void TitleState::DrawUI()  {
    float bgZ   = DrawingUtilities::GetDrawingZ(DrawOrder::BACKGROUND);
    //float uiZ   = DrawingUtilities::GetDrawingZ(DrawOrder::UI_OBJECTS);
    //float menuZ = DrawingUtilities::GetDrawingZ(DrawOrder::MENUTEXT);

    _background.Draw(bgZ);

    mNexusImage->Draw();
    /*
    mDoorGlow->Draw();
    mDoorSpin1->Draw();
    mDoorSpin2->Draw();
    mTitleTex->Draw();
    mTitleOverlay->Draw();

    mSubtitle->Draw();
    mSubtitleOverlay->Draw();
*/

    //The UI labels get drawn in the TitleScreen overlay
    /*
    if (_pressEnterVisible && _pressEnterTex && !GlobalState::flash.Active()) {
        int px = (160 - _pressEnterTex->Width) / 2;
        Rectangle dst{px, 160, _pressEnterTex->Width, _pressEnterTex->Height};
        SpriteDrawer::DrawSprite(_pressEnterTex, dst, nullptr, nullptr, 0.f,
                                 SpriteEffects::None, menuZ);
    }
    */
}
}
