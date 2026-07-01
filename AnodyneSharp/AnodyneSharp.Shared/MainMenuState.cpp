#include "MainMenuState.hpp"
#include "PlayState.hpp"
#include "TitleState.hpp"
#include "SpriteDrawer.hpp"
#include <iostream>

namespace AnodyneSharp
{
namespace States
{
namespace MainMenu
{
    MainMenuState::MainMenuState(IStateSetter* stateSetter)
        : _stateSetter(stateSetter)
    {
        _options = { "Start Game", "Config", "Exit" };
        _dirty = true;
    }

    void MainMenuState::Create()
    {
        RefreshMenu();
    }

    void MainMenuState::Update()
    {
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up))
        {
            if (_selected > 0)
            {
                --_selected;
                std::cout << "Selected: " << _options[_selected] << "\n";
                _dirty = true;
            }
        }
        else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Down))
        {
            if (_selected + 1 < _options.size())
            {
                ++_selected;
                std::cout << "Selected: " << _options[_selected] << "\n";
                _dirty = true;
            }
        }
        else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept))
        {
            if (_selected == 0)
            {
                std::cout << "Starting game...\n";
                _stateSetter->ChangeState(std::make_unique<States::Gameplay::PlayState>(_stateSetter));
            }
            else if (_selected == 1)
            {
                std::cout << "Config selected. (Not implemented yet)\n";
            }
            else if (_selected == 2)
            {
                _stateSetter->ExitGame();
            }
        }
        else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel))
        {
            std::cout << "Returning to title...\n";
            _stateSetter->ChangeState(std::make_unique<States::MainMenu::TitleState>(_stateSetter));
        }
    }

    void MainMenuState::Draw()
    {
        // Draw is intentionally empty for this placeholder UI.
    }

    void MainMenuState::RefreshMenu()
    {
        std::cout << "=== Main Menu ===\n";
        for (size_t i = 0; i < _options.size(); ++i)
        {
            std::cout << (i == _selected ? "> " : "  ") << _options[i] << "\n";
        }
        std::cout << "=================\n";
    }

    void MainMenuState::DrawUI()
    {
        if (_dirty)
        {
            RefreshMenu();
            _dirty = false;
        }

        SpriteDrawer::DrawText("MAIN MENU", 24, 16, Microsoft::Xna::Framework::Color::White(), 4);
        int y = 80;
        for (size_t i = 0; i < _options.size(); ++i)
        {
            std::string line = (i == _selected ? "> " : "  ") + _options[i];
            SpriteDrawer::DrawText(line, 24, y, Microsoft::Xna::Framework::Color::White(), 2);
            y += 16;
        }
    }
}
}
}
