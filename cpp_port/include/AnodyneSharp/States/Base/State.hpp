#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp::States {

class State {
public:
    bool Exit              = false;
    bool UpdateEntities    = true;
    bool DrawPlayState     = true;

    virtual ~State() = default;
    virtual void Create()     {}
    virtual void Initialize() {}
    virtual void Update()     {}
    virtual void Draw()       {}
    virtual void DrawUI()     {}
};

class IStateSetter {
public:
    virtual ~IStateSetter() = default;
    virtual void SetStateByName(const std::string& typeName) = 0;
    // Template wrapper
    template<typename T>
    void SetState() { SetStateImpl([]{ return std::make_unique<T>(); }); }
protected:
    virtual void SetStateImpl(std::function<std::unique_ptr<State>()> factory) = 0;
};

} // namespace AnodyneSharp::States

using AnodyneSharp::States::State;
using AnodyneSharp::States::IStateSetter;
