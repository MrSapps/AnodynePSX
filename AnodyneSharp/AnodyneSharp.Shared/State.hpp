#pragma once

#include <memory>

// TODO: State base class is a simplified port. Complete this from the C# state machine in AnodyneGame.cs.

namespace AnodyneSharp
{
    class State
    {
    public:
        virtual ~State() = default;
        virtual void Create() {}
        virtual void Update() {}
        virtual void Draw() {}
        virtual void DrawUI() {}
    };

    class IStateSetter
    {
    public:
        virtual ~IStateSetter() = default;
        virtual void ChangeState(std::unique_ptr<State> newState) = 0;
        virtual void ExitGame() = 0;
    };
}
