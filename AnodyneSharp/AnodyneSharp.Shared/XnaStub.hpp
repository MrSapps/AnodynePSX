#pragma once

#include <cstdint>
#include <string>

// TODO: This XNA stub is incomplete and should continue porting missing C# MonoGame behavior from AnodyneGame.cs and related XNA classes.

namespace Microsoft {
namespace Xna {
namespace Framework
{
    struct Vector2
    {
        float X = 0.0f;
        float Y = 0.0f;

        constexpr Vector2(float x = 0.0f, float y = 0.0f) : X(x), Y(y) {}
    };

    struct Color
    {
        uint8_t R = 255;
        uint8_t G = 255;
        uint8_t B = 255;
        uint8_t A = 255;

        constexpr Color(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255)
            : R(r), G(g), B(b), A(a)
        {
        }
            static Color White();
            static Color LightBlue();
    };

    struct GameTime
    {
        double TotalSeconds = 0.0;
        double ElapsedSeconds = 0.0;
    };

    struct GameWindow
    {
        std::string Title;
    };

    class Game
    {
    public:
        Game() = default;
        virtual ~Game() = default;

        virtual void Initialize() {}
        virtual void LoadContent() {}
        virtual void Update(const GameTime&) {}
        virtual void Draw(const GameTime&) {}

        int Run();

        virtual bool IsActive() const { return true; }
        virtual void Exit();

        bool IsExitRequested = false;

        bool IsFixedTimeStep = false;

        GameWindow Window;
    };

namespace Graphics
{
    enum class GraphicsProfile
    {
        Reach,
        HiDef
    };

    struct GraphicsDevice
    {
        struct PresentationParameters
        {
            int BackBufferWidth = 0;
            int BackBufferHeight = 0;
        };

        PresentationParameters PresentationParameters;
        void* Renderer = nullptr;
        void* Window = nullptr;

        void Clear(const Microsoft::Xna::Framework::Color&);
        void Present();
    };

    struct GraphicsDeviceManager
    {
        GraphicsDevice* Device = nullptr;
        GraphicsProfile Profile = GraphicsProfile::Reach;
        bool IsFullScreen = false;
        int PreferredBackBufferWidth = 0;
        int PreferredBackBufferHeight = 0;
        bool SynchronizeWithVerticalRetrace = false;

        explicit GraphicsDeviceManager(Microsoft::Xna::Framework::Game*);
        ~GraphicsDeviceManager();

        void ApplyChanges();
        GraphicsDevice* GetGraphicsDevice() { return Device; }
        const GraphicsDevice* GetGraphicsDevice() const { return Device; }
    };

    struct ContentManager
    {
        std::string RootDirectory;
    };
    }
    }
    }
}
