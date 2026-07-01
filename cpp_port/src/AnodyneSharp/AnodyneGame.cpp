#include "AnodyneSharp/AnodyneGame.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Drawing/Effects/Effects.hpp"
#include "AnodyneSharp/Input/KeyInput.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/Entities/EntityManager.hpp"
#include "AnodyneSharp/Dialogue/DialogueManager.hpp"
#include "SDL3Context.hpp"
#include <format>

// Static member definition (defined outside namespace)
bool AnodyneSharp::Drawing::Effects::ScreenShake::Enabled = true;

namespace AnodyneSharp {

AnodyneGame::AnodyneGame() {
    GlobalState::GameState = this;
    ModLoader::Initialize();
}

void AnodyneGame::Initialize() {
    // Set content root so ResourceManager and Map.cpp find assets
    ResourceManager::BaseDir =
        "AnodyneSharp/AnodyneSharp/AnodyneSharp.Shared";

    // Wire up quick-save / quick-load function pointers
    GlobalState::DoQuickSave = []() {
        GlobalState::SaveGame(GlobalState::CurrentSaveGame);
    };
    GlobalState::DoQuickLoad = []() {
        GlobalState::LoadSave(GlobalState::CurrentSaveGame);
    };

    GlobalState::ResetValues();
    Input::KeyInput::InitDefaults();
    Dialogue::DialogueManager::Reload();
    EntityManager::Initialize();
}

void AnodyneGame::LoadContent() {
    ResourceManager::LoadResources(_content);
    SDL_Log("LoadContent: resources loaded, now setting TitleState");
    Drawing::SpriteDrawer::Load(_content);
    SetState<States::TitleState>();
    SDL_Log("LoadContent: TitleState set");
}

void AnodyneGame::Update(const GameTime& gameTime) {
    // Apply deferred state deletion — the old state may have triggered SetState from within its own Update,
    // so we must not delete it until we're fully back in the main loop.
    if (_stateToDelete) {
        SDL_Log("AnodyneGame::Update: deleting old state %p, new=%p", (void*)_stateToDelete, (void*)CurrentState);
        delete _stateToDelete;
        _stateToDelete = nullptr;
    }

    GameTimes::TimeScale = 1.f;

    GameTimes::UpdateTimes(gameTime);
    Input::KeyInput::Update();

    // Sync screenshake enabled flag from settings (no GlobalState include in Effects.hpp)
    Drawing::Effects::ScreenShake::Enabled = GlobalState::settings.screenshake;

    if (!GlobalState::settings.pause_on_unfocus || IsActive) {
        if (CurrentState) CurrentState->Update();

        for (auto* e : GlobalState::AllEffects()) {
            if (e->Active()) e->Update();
        }

        if (Input::KeyInput::JustPressedKey(Keys::F12)) {
            GlobalState::ShowFPS = !GlobalState::ShowFPS;
        }

        if (GlobalState::ClosingGame) Exit();

        if (GlobalState::ResolutionDirty) {
            InitGraphics();
            GlobalState::ResolutionDirty = false;
        }

        for (auto& mod : ModLoader::mods) mod->Update();
    }
}

void AnodyneGame::Draw(const GameTime& gameTime) {
    GameTimes::UpdateFPS(gameTime);

    if (GlobalState::ShowFPS && _fpsLabel)
        _fpsLabel->SetText(std::format("FPS: {:.0f}", GameTimes::FPS));

    Drawing::SpriteDrawer::BeginDraw();
    // Shake: displace world draw by a fraction of screen dimensions
    if (GlobalState::screenShake.Active()) {
        Drawing::SpriteDrawer::_camOffset.X += GlobalState::screenShake.Offset.X * 160.f;
        Drawing::SpriteDrawer::_camOffset.Y += GlobalState::screenShake.Offset.Y * 180.f;
    }
    if (CurrentState) CurrentState->Draw();
    Drawing::SpriteDrawer::EndDraw();

    Drawing::SpriteDrawer::BeginGUIDraw();
    if (CurrentState) CurrentState->DrawUI();
    if (GlobalState::ShowFPS && _fpsLabel) _fpsLabel->Draw();

    // Darkness overlay (flat black, scaled by darkness.Alpha)
    if (GlobalState::darkness.Active() && Drawing::SpriteDrawer::SolidTex) {
        uint8_t alpha = static_cast<uint8_t>(GlobalState::darkness.Alpha * 255.f);
        Color darkColor(0, 0, 0, (int)alpha);
        Rectangle full{0, 0, 160, 180};
        Drawing::SpriteDrawer::DrawSprite(
            Drawing::SpriteDrawer::SolidTex, full, nullptr,
            &darkColor, 0.f, SpriteEffects::None,
            Drawing::DrawingUtilities::GetDrawingZ(Drawing::DrawOrder::DARKNESS));
    }

    // black_overlay / gameScreenFade — flat black at varying alpha (map transitions, death)
    auto drawFade = [&](const Drawing::Effects::FadeEffect& fe, Drawing::DrawOrder zOrder) {
        if (fe.Active() && fe.alpha > 0.f && Drawing::SpriteDrawer::SolidTex) {
            Color c(0, 0, 0, (int)(fe.alpha * 255.f));
            Rectangle full{0, 0, 160, 180};
            Drawing::SpriteDrawer::DrawSprite(
                Drawing::SpriteDrawer::SolidTex, full, nullptr, &c,
                0.f, SpriteEffects::None,
                Drawing::DrawingUtilities::GetDrawingZ(zOrder));
        }
    };
    drawFade(GlobalState::black_overlay,   Drawing::DrawOrder::BLACK_OVERLAY);
    drawFade(GlobalState::gameScreenFade,  Drawing::DrawOrder::BLACK_OVERLAY);

    Drawing::SpriteDrawer::EndGUIDraw();
    Drawing::SpriteDrawer::Render();
}

void AnodyneGame::SetStateImpl(std::function<std::unique_ptr<States::State>()> factory) {
    for (auto* e : GlobalState::AllEffects()) e->Deactivate();
    _stateToDelete = CurrentState;  // defer deletion — may be on call stack right now
    CurrentState = factory().release();
    SDL_Log("SetStateImpl: new state=%p (old deferred)", (void*)CurrentState);
    CurrentState->Create();
}

void AnodyneGame::InitGraphics() {
    if (!SDL3Context::Renderer) return;
    auto& s = GlobalState::settings;
    int scale = s.scale > 0 ? s.scale : 4;
    SDL_SetWindowSize(SDL3Context::Window,
        SDL3Context::GAME_W * scale,
        SDL3Context::GAME_H * scale);
    bool fullscreen = (s.resolution != Registry::Resolution::Windowed);
    SDL_SetWindowFullscreen(SDL3Context::Window, fullscreen);
    SDL_SetRenderLogicalPresentation(
        SDL3Context::Renderer,
        SDL3Context::GAME_W, SDL3Context::GAME_H,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
}

void AnodyneGame::SetDefaultKeys() {
    using namespace Input;
    KeyInput::RebindableKeys[KeyFunctions::Up]   = RebindableKey({Keys::Up, Keys::W});
    KeyInput::RebindableKeys[KeyFunctions::Down]  = RebindableKey({Keys::Down, Keys::S});
    KeyInput::RebindableKeys[KeyFunctions::Left]  = RebindableKey({Keys::Left, Keys::A});
    KeyInput::RebindableKeys[KeyFunctions::Right] = RebindableKey({Keys::Right, Keys::D});
    KeyInput::RebindableKeys[KeyFunctions::Accept] = RebindableKey({Keys::C});
    KeyInput::RebindableKeys[KeyFunctions::Cancel] = RebindableKey({Keys::X, Keys::Space});
    KeyInput::RebindableKeys[KeyFunctions::Pause]  = RebindableKey({Keys::Enter, Keys::Escape});
}

} // namespace AnodyneSharp
