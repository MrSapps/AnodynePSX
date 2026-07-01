#pragma once

#include "State.hpp"
#include "KeyInput.hpp"
#include <string>
#include <vector>

namespace AnodyneSharp
{
namespace States
{
namespace MainMenu
{
    class MainMenuState : public State
    {
    public:
        explicit MainMenuState(IStateSetter* stateSetter);
        void Create() override;
        void Update() override;
        void Draw() override;
        void DrawUI() override;

    private:
        IStateSetter* _stateSetter;
        std::vector<std::string> _options;
        size_t _selected = 0;
        bool _dirty = true;

        void RefreshMenu();
    };
}
}
}
