#include "PlayState.hpp"
#include "TitleState.hpp"
#include <iostream>

// TODO: Port the full PlayState logic from States/PlayState.cs.

namespace AnodyneSharp
{
namespace States
{
namespace Gameplay
{
    PlayState::PlayState(IStateSetter* stateSetter)
        : _stateSetter(stateSetter)
    {
        // TODO: implement gameplay state initialization, map loading, and entity update logic from the original C# class.
    }

    void PlayState::Create()
    {
        std::cout << "Gameplay started. Press Escape to exit." << std::endl;
    }

    void PlayState::Update()
    {
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Pause) || KeyInput::JustPressedKey(Keys::Escape))
        {
            _stateSetter->ChangeState(std::make_unique<States::MainMenu::TitleState>(_stateSetter));
        }
    }

    void PlayState::Draw()
    {
        // TODO: render gameplay world, map, player, and entities based on PlayState.cs.
    }

    void PlayState::DrawUI()
    {
    }
}
}
}
