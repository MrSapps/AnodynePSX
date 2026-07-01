#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Entities/Base/EntityPreset.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/MapData/Map.hpp"
#include "AnodyneSharp/Entities/CollisionGroups.hpp"
#include "AnodyneSharp/Entities/EntityManager.hpp"

// All entity includes consolidated

namespace AnodyneSharp::States {

// State machine substate base
class Substate {
public:
    virtual ~Substate() = default;
    virtual void Update() {}
    virtual void Draw()   {}
    virtual void DrawUI() {}
    bool Exit = false;
};

// IStateSetter interface
class IStateSetter {
public:
    virtual ~IStateSetter() = default;
    template<typename T>
    void SetState() { SetStateImpl([]{ return std::make_unique<T>(); }); }
protected:
    virtual void SetStateImpl(std::function<std::unique_ptr<class State>()> factory) = 0;
};

} // namespace AnodyneSharp::States
