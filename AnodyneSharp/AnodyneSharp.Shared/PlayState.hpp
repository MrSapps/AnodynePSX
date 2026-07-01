#pragma once

#include "State.hpp"
#include "KeyInput.hpp"

namespace AnodyneSharp
{
namespace States
{
namespace Gameplay
{
    class PlayState : public State
    {
    public:
        explicit PlayState(IStateSetter* stateSetter);
        void Create() override;
        void Update() override;
        void Draw() override;
        void DrawUI() override;

    private:
        IStateSetter* _stateSetter;
    };
}
}
}
