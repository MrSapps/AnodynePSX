#include "XnaStub.hpp"
#include "SpriteDrawer.hpp"
#include "State.hpp"
#include "KeyInput.hpp"
#include "TitleState.hpp"
#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>

// TODO: This file is a partial bootstrap port of AnodyneGame.cs.
// The current C++ implementation is intentionally minimal and stubbed.
// Porting checklist:
// - content loading and ContentManager asset pipeline
// - input mapping, key rebinding, and controller support
// - save file handling and file system initialization
// - title screen / state machine / menu transitions
// - dialog manager, effects, and mod loading
// - cheat handling and debug toggles
// - actual rendering via SpriteDrawer / GraphicsDevice

namespace AnodyneSharp
{
    enum class Resolution
    {
        Windowed,
        Scaled,
        Stretch
    };

    enum class FPS
    {
        Fixed,
        VSync,
        Unlocked
    };

    enum class Buttons
    {
        DPadUp,
        LeftThumbstickUp,
        DPadRight,
        LeftThumbstickRight,
        DPadDown,
        LeftThumbstickDown,
        DPadLeft,
        LeftThumbstickLeft,
        A,
        B,
        Start,
        Back,
        LeftShoulder,
        LeftTrigger,
        RightShoulder,
        RightTrigger,
        RightThumbstickUp,
        RightThumbstickRight,
        RightThumbstickDown,
        RightThumbstickLeft
    };

    struct GameConstants
    {
        static constexpr const char* SavePath = "Save/";
        static constexpr int HEADER_HEIGHT = 18;
    };

    // TODO: port the full state machine and all states from AnodyneGame.cs.

    class UILabel
    {
    public:
        UILabel(const Microsoft::Xna::Framework::Vector2& /*position*/, bool /*drawShadow*/, const std::string& /*text*/, const Microsoft::Xna::Framework::Color& /*color*/, int /*layer*/ = 0, bool /*forceEnglish*/ = false, bool /*centerText*/ = false)
        {
        }

        // TODO: implement font/text layout, styling, and drawing from AnodyneGame.cs and related UI code.

        void SetText(const std::string& /*text*/)
        {
        }

        void Draw() const
        {
        }
    };

    // SpriteDrawer implementation moved to SpriteDrawer.hpp/cpp.

    class EntityManager
    {
    public:
        static void Initialize() {}
        // TODO: port entity manager initialization and entity registration from AnodyneGame.cs / EntityManager.cs.
    };

    // KeyInput is implemented in KeyInput.hpp/cpp.

    class GlobalState
    {
    public:
        static IStateSetter* GameState;
        static bool ClosingGame;
        static bool ResolutionDirty;
        static bool ShowFPS;

        struct Settings
        {
            bool pause_on_unfocus = false;
            Resolution resolution = Resolution::Windowed;
            int scale = 1;
            FPS fps = FPS::Fixed;
        };

        static Settings settings;

        static void ResetValues()
        {
            settings.pause_on_unfocus = false;
            settings.resolution = Resolution::Windowed;
            settings.scale = 1;
            settings.fps = FPS::Fixed;
            ClosingGame = false;
            ResolutionDirty = false;
            ShowFPS = false;
        }
    };

    IStateSetter* GlobalState::GameState = nullptr;
    bool GlobalState::ClosingGame = false;
    bool GlobalState::ResolutionDirty = false;
    bool GlobalState::ShowFPS = false;
    GlobalState::Settings GlobalState::settings;

    class AnodyneGame : public Microsoft::Xna::Framework::Game, public IStateSetter
    {
    public:
        AnodyneGame()
            : graphics(this)
        {
            // TODO: port culture initialization and mod loader from AnodyneGame.cs and modding startup code.
            Content.RootDirectory = "Content";
            CurrentState = nullptr;
            GlobalState::GameState = this;

            // TODO: port input config and default key bindings from AnodyneGame.cs.
            SetDefaultKeys();

                // Create save directories in the same structure as AnodyneGame.cs.
                std::filesystem::create_directories(GameConstants::SavePath);
                std::filesystem::create_directories(std::string(GameConstants::SavePath) + "Saves/");

                // TODO: port the C# save file and input config loader/writer logic.

#if WINDOWS
            InitGraphics();
#endif
        }

        void Initialize() override
        {
            InitGraphics();

            SpriteDrawer::Initialize(graphics.GetGraphicsDevice());
            GlobalState::ResetValues();
            EntityManager::Initialize();

            // TODO: call base.Initialize() as done in AnodyneGame.cs.
            _fpsLabel = std::make_unique<UILabel>(Microsoft::Xna::Framework::Vector2(0.0f, GameConstants::HEADER_HEIGHT), false, "", Microsoft::Xna::Framework::Color::LightBlue());

            // TODO: Window.Title = "Anodyne Fan Remake"; (from AnodyneGame.cs window setup)
            if (graphics.GetGraphicsDevice() && graphics.GetGraphicsDevice()->Window)
            {
                SDL_SetWindowTitle(static_cast<SDL_Window*>(graphics.GetGraphicsDevice()->Window), "Anodyne Fan Remake");
            }
            SetState<States::MainMenu::TitleState>(this);
        }

        void LoadContent() override
        {
            // TODO: port content loading, input configuration, resource manager, dialogue manager,
            // and effect loading from AnodyneGame.cs and the MonoGame content pipeline.
            SpriteDrawer::Load(Content);
            const std::string inputConfigPath = std::string(GameConstants::SavePath) + "InputConfig.dat";
            if (std::filesystem::exists(inputConfigPath))
            {
                if (!KeyInput::LoadInputConfig(inputConfigPath))
                {
                    SetDefaultKeys();
                }
            }
            else
            {
                SetDefaultKeys();
                KeyInput::SaveInputConfig(inputConfigPath);
            }
        }
        void Update(const Microsoft::Xna::Framework::GameTime& /*gameTime*/) override
        {
            // TODO: call base.Update(gameTime) and update GameTimes like AnodyneGame.cs.
            KeyInput::Update();

            if (GlobalState::settings.pause_on_unfocus && !IsActive())
            {
                return;
            }

            if (CurrentState)
            {
                CurrentState->Update();
            }

            // TODO: update effects and mods from the original C# update loop.

            if (GlobalState::ClosingGame)
            {
                Exit();
            }

            if (GlobalState::ResolutionDirty)
            {
                InitGraphics();
                GlobalState::ResolutionDirty = false;
            }
        }

        void Draw(const Microsoft::Xna::Framework::GameTime& /*gameTime*/) override
        {
            // TODO: call base.Draw(gameTime) and update FPS timer as in AnodyneGame.cs.
            if (GlobalState::ShowFPS && _fpsLabel)
            {
                _fpsLabel->SetText("FPS: 0");
            }

#if DEBUG
            if (KeyInput::JustPressedKey(Keys::F11))
            {
                // TODO: port cheat toggle from AnodyneGame.cs / cheat input flow.
            }
#endif

            SpriteDrawer::BeginDraw();
            if (CurrentState)
            {
                CurrentState->Draw();
            }
            SpriteDrawer::EndDraw();

            SpriteDrawer::BeginGUIDraw();
            if (CurrentState)
            {
                CurrentState->DrawUI();
            }

            if (GlobalState::ShowFPS && _fpsLabel)
            {
                _fpsLabel->Draw();
            }

            SpriteDrawer::EndGUIDraw();
            SpriteDrawer::Render();
        }

        void ChangeState(std::unique_ptr<State> newState) override
        {
            CurrentState = std::move(newState);
            if (CurrentState)
            {
                CurrentState->Create();
            }
        }

        void ExitGame() override
        {
            Exit();
        }

        template <typename T, typename... Args>
        void SetState(Args&&... args)
        {
            CurrentState = std::make_unique<T>(std::forward<Args>(args)...);
            if (CurrentState)
            {
                CurrentState->Create();
            }
        }

    private:
        void InitGraphics()
        {
            if (GlobalState::settings.resolution == Resolution::Windowed)
            {
                graphics.PreferredBackBufferWidth = 160 * GlobalState::settings.scale;
                graphics.PreferredBackBufferHeight = 180 * GlobalState::settings.scale;
                graphics.IsFullScreen = false;
            }
            else
            {
                graphics.PreferredBackBufferWidth = 160 * GlobalState::settings.scale;
                graphics.PreferredBackBufferHeight = 180 * GlobalState::settings.scale;
                graphics.IsFullScreen = false;
            }

            switch (GlobalState::settings.fps)
            {
            case FPS::Fixed:
                IsFixedTimeStep = true;
                graphics.SynchronizeWithVerticalRetrace = true;
                break;
            case FPS::VSync:
                IsFixedTimeStep = false;
                graphics.SynchronizeWithVerticalRetrace = true;
                break;
            case FPS::Unlocked:
                IsFixedTimeStep = false;
                graphics.SynchronizeWithVerticalRetrace = false;
                break;
            }

            graphics.ApplyChanges();
        }

        // TODO: port input configuration and rebindable key handling from AnodyneGame.cs input config.
        void SetDefaultKeys()
        {
            // TODO: mirror the input configuration and rebindable keys from AnodyneGame.cs.
            KeyInput::RebindableKeys = {
                { KeyFunctions::Up, { { Keys::Up, Keys::W } } },
                { KeyFunctions::Right, { { Keys::Right, Keys::D } } },
                { KeyFunctions::Down, { { Keys::Down, Keys::S } } },
                { KeyFunctions::Left, { { Keys::Left, Keys::A } } },
                { KeyFunctions::Accept, { { Keys::C, Keys::Right } } },
                { KeyFunctions::Cancel, { { Keys::X, Keys::Space } } },
                { KeyFunctions::Pause, { { Keys::Enter, Keys::Escape } } },
                { KeyFunctions::PreviousPage, { { Keys::PageDown } } },
                { KeyFunctions::NextPage, { { Keys::PageUp } } },
                { KeyFunctions::Broom1, { { Keys::D1 } } },
                { KeyFunctions::Broom2, { { Keys::D2 } } },
                { KeyFunctions::Broom3, { { Keys::D3 } } },
                { KeyFunctions::Broom4, { { Keys::D4 } } },
                { KeyFunctions::QuickSave, { { Keys::F4 } } },
                { KeyFunctions::QuickLoad, { { Keys::F5 } } }
            };
        }

    private:
        Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager graphics;
        Microsoft::Xna::Framework::Graphics::ContentManager Content;
        std::unique_ptr<State> CurrentState;
        std::unique_ptr<UILabel> _fpsLabel;
        bool IsFixedTimeStep = false;
    };
}

int main()
{
    AnodyneSharp::AnodyneGame game;
    return game.Run();
}
