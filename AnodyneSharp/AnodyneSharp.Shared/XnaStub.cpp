#include "XnaStub.hpp"
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>

namespace Microsoft {
namespace Xna {
namespace Framework
{
    Color Color::White()
    {
        return Color(255, 255, 255, 255);
    }

    Color Color::LightBlue()
    {
        return Color(173, 216, 230, 255);
    }

    int Game::Run()
    {
        using namespace std::chrono;
        Initialize();
        LoadContent();

        auto last = steady_clock::now();
        GameTime gameTime;

        while (!IsExitRequested)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    IsExitRequested = true;
                }
            }
            auto now = steady_clock::now();
            gameTime.ElapsedSeconds = duration<double>(now - last).count();
            last = now;
            gameTime.TotalSeconds += gameTime.ElapsedSeconds;

            Update(gameTime);
            Draw(gameTime);

            std::this_thread::sleep_for(milliseconds(16));
        }

        return 0;
    }

    void Game::Exit()
    {
        IsExitRequested = true;
    }

    namespace Graphics
    {
        GraphicsDeviceManager::GraphicsDeviceManager(Microsoft::Xna::Framework::Game*)
            : Device(new GraphicsDevice())
        {
            if (SDL_Init(SDL_INIT_VIDEO) == 0)
            {
                if (Device->PresentationParameters.BackBufferWidth <= 0)
                {
                    Device->PresentationParameters.BackBufferWidth = 800;
                }
                if (Device->PresentationParameters.BackBufferHeight <= 0)
                {
                    Device->PresentationParameters.BackBufferHeight = 600;
                }
                Device->Window = SDL_CreateWindow("Anodyne Fan Remake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Device->PresentationParameters.BackBufferWidth, Device->PresentationParameters.BackBufferHeight, SDL_WINDOW_SHOWN);
                if (Device->Window)
                {
                    Device->Renderer = SDL_CreateRenderer(static_cast<SDL_Window*>(Device->Window), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                }
            }
        }

        GraphicsDeviceManager::~GraphicsDeviceManager()
        {
            if (Device)
            {
                if (Device->Renderer)
                {
                    SDL_DestroyRenderer(static_cast<SDL_Renderer*>(Device->Renderer));
                    Device->Renderer = nullptr;
                }
                if (Device->Window)
                {
                    SDL_DestroyWindow(static_cast<SDL_Window*>(Device->Window));
                    Device->Window = nullptr;
                }
                delete Device;
                Device = nullptr;
            }
            SDL_Quit();
        }

        void GraphicsDeviceManager::ApplyChanges()
        {
            if (!Device)
            {
                return;
            }

            if (!Device->Window)
            {
                Device->Window = SDL_CreateWindow("Anodyne Fan Remake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Device->PresentationParameters.BackBufferWidth, Device->PresentationParameters.BackBufferHeight, SDL_WINDOW_SHOWN);
                if (Device->Window)
                {
                    Device->Renderer = SDL_CreateRenderer(static_cast<SDL_Window*>(Device->Window), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                }
                return;
            }

            SDL_SetWindowSize(static_cast<SDL_Window*>(Device->Window), Device->PresentationParameters.BackBufferWidth, Device->PresentationParameters.BackBufferHeight);
        }

        void GraphicsDevice::Clear(const Microsoft::Xna::Framework::Color& color)
        {
            if (!Renderer)
            {
                return;
            }

            SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(Renderer), color.R, color.G, color.B, color.A);
            SDL_RenderClear(static_cast<SDL_Renderer*>(Renderer));
        }

        void GraphicsDevice::Present()
        {
            if (!Renderer)
            {
                return;
            }
            SDL_RenderPresent(static_cast<SDL_Renderer*>(Renderer));
        }
    }
}
}
}
