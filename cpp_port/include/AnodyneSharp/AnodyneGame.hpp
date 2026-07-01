#pragma once
#include "AnodyneSharp/States/AllStates.hpp"
#include "AnodyneSharp/States/Base/State.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Input/KeyInput.hpp"
#include "AnodyneSharp/Modding/ModAndCheatz.hpp"
#include "AnodyneSharp/UI/UITypes.hpp"
#include "AnodyneSharp/Entities/EntityManager.hpp"
#include "XNA/Framework.hpp"

namespace AnodyneSharp {

class AnodyneGame : public Game, public States::IStateSetter {
public:
    AnodyneGame();

    States::State* CurrentState = nullptr;
    States::State* _stateToDelete = nullptr; // deferred deletion: set in SetStateImpl, freed after Update returns

protected:
    void Initialize()    override;
    void LoadContent()   override;
    void Update(const GameTime& gameTime) override;
    void Draw(const GameTime& gameTime)   override;

    void SetStateImpl(std::function<std::unique_ptr<States::State>()> factory) override;
    void SetStateByName(const std::string& typeName) override {}

public:
    template<typename T>
    void SetState() {
        // Deactivate all effects
        for (auto* e : GlobalState::AllEffects())
            e->Deactivate();
        CurrentState = new T();
        CurrentState->Create();
    }

private:
    GraphicsDeviceManager _graphics;
    ContentManager _content;
    std::unique_ptr<UI::UILabel> _fpsLabel;

    void InitGraphics();
    void SetDefaultKeys();
};

} // namespace AnodyneSharp

using AnodyneSharp::AnodyneGame;
