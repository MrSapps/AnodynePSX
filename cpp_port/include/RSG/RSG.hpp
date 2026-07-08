#pragma once
// RSG (Robot State Machine) C++ port
// Mirrors the C# RSG state machine library API used by AnodyneSharp

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <SDL3/SDL.h>

namespace RSG {

class IState;

// ---------------------------------------------------------------
// AbstractState
// ---------------------------------------------------------------
class AbstractState {
public:
    virtual ~AbstractState() = default;

    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void Update(float deltaTime) {}

    void TriggerEvent(const std::string& name);
    template<typename T>
    void TriggerEvent(const std::string& name, T eventData);

    void ChangeState(const std::string& name);

    // Internal - set by state machine
    std::function<void(const std::string&)> _changeStateFn;
    std::function<void(const std::string&, std::shared_ptr<void>)> _triggerFn;
};

// ---------------------------------------------------------------
// IState interface
// ---------------------------------------------------------------
class IState {
public:
    virtual ~IState() = default;
    virtual void Update(float deltaTime) = 0;
    virtual void ChangeState(const std::string& stateName) = 0;
    virtual void TriggerEvent(const std::string& eventName) = 0;
    virtual void TriggerEvent(const std::string& eventName, std::shared_ptr<void> data) = 0;
};

// ---------------------------------------------------------------
// EventArgs base
// ---------------------------------------------------------------
struct EventArgs {
    virtual ~EventArgs() = default;
};

// ---------------------------------------------------------------
// Internal state node
// ---------------------------------------------------------------
struct StateNode {
    std::string name;
    std::shared_ptr<AbstractState> stateObj;

    std::function<void(AbstractState*)> enterFn;
    std::function<void(AbstractState*)> exitFn;
    std::function<void(AbstractState*, float)> updateFn;

    struct EventHandler {
        std::function<void(AbstractState*)> fn;
        std::function<void(AbstractState*, std::shared_ptr<void>)> fnWithData;
    };
    std::unordered_map<std::string, EventHandler> events;

    struct ConditionEntry {
        std::function<bool()> condition;
        std::function<void(AbstractState*)> action;
    };
    std::vector<ConditionEntry> conditions;
};

// ---------------------------------------------------------------
// StateMachine implementation
// ---------------------------------------------------------------
class StateMachine : public IState {
    std::unordered_map<std::string, StateNode> _states;
    std::string _currentStateName;
    StateNode* _currentState = nullptr;

public:
    StateMachine(std::unordered_map<std::string, StateNode>&& states)
        : _states(std::move(states)) {}

    void ChangeState(const std::string& stateName) override {
        if (_currentState && _currentState->exitFn) {
            _currentState->exitFn(_currentState->stateObj.get());
        }
        auto it = _states.find(stateName);
        if (it == _states.end()) {
            throw std::runtime_error("State not found: " + stateName);
        }

        SDL_Log("StateMachine: Changing state from %s to %s", _currentStateName.c_str(), stateName.c_str());
        _currentStateName = stateName;
        _currentState = &it->second;

        // Wire up change/trigger
        auto* sm = this;
        if (_currentState->stateObj) {
            _currentState->stateObj->_changeStateFn = [sm](const std::string& n){ sm->ChangeState(n); };
            _currentState->stateObj->_triggerFn = [sm](const std::string& n, std::shared_ptr<void> d){ sm->TriggerEvent(n, d); };
        }

        if (_currentState->enterFn) {
            _currentState->enterFn(_currentState->stateObj.get());
        }
    }

    void Update(float deltaTime) override {
        if (!_currentState) return;
        if (_currentState->stateObj) {
            _currentState->stateObj->Update(deltaTime);
        }
        if (_currentState->updateFn) {
            _currentState->updateFn(_currentState->stateObj.get(), deltaTime);
        }
        for (auto& cond : _currentState->conditions) {
            if (cond.condition()) {
                cond.action(_currentState->stateObj.get());
                break; // only first matching
            }
        }
    }

    void TriggerEvent(const std::string& eventName) override {
        TriggerEvent(eventName, nullptr);
    }

    void TriggerEvent(const std::string& eventName, std::shared_ptr<void> data) override {
        if (!_currentState) return;
        auto it = _currentState->events.find(eventName);
        if (it != _currentState->events.end()) {
            if (data && it->second.fnWithData) {
                it->second.fnWithData(_currentState->stateObj.get(), data);
            } else if (it->second.fn) {
                it->second.fn(_currentState->stateObj.get());
            }
        }
    }
};

// Inline implementations of AbstractState
inline void AbstractState::TriggerEvent(const std::string& name) {
    if (_triggerFn) _triggerFn(name, nullptr);
}
template<typename T>
inline void AbstractState::TriggerEvent(const std::string& name, T eventData) {
    if (_triggerFn) _triggerFn(name, std::make_shared<T>(std::move(eventData)));
}
inline void AbstractState::ChangeState(const std::string& name) {
    if (_changeStateFn) _changeStateFn(name);
}

// ---------------------------------------------------------------
// StateBuilder (fluent builder)
// ---------------------------------------------------------------
class StateMachineBuilder;

class StateBuilder {
    StateNode _node;
    StateMachineBuilder* _parent;
    std::string _name;

public:
    StateBuilder(StateMachineBuilder* parent, const std::string& name,
                 std::shared_ptr<AbstractState> stateObj = nullptr)
        : _parent(parent), _name(name) {
        _node.name = name;
        _node.stateObj = stateObj ? stateObj : std::make_shared<AbstractState>();
    }

    StateBuilder& Enter(std::function<void(AbstractState*)> fn) {
        _node.enterFn = fn;
        return *this;
    }
    // Typed enter (for typed states)
    template<typename T>
    StateBuilder& Enter(std::function<void(T*)> fn) {
        _node.enterFn = [fn](AbstractState* s){ fn(static_cast<T*>(s)); };
        return *this;
    }

    StateBuilder& Exit(std::function<void(AbstractState*)> fn) {
        _node.exitFn = fn;
        return *this;
    }
    template<typename T>
    StateBuilder& Exit(std::function<void(T*)> fn) {
        _node.exitFn = [fn](AbstractState* s){ fn(static_cast<T*>(s)); };
        return *this;
    }

    StateBuilder& Update(std::function<void(AbstractState*, float)> fn) {
        _node.updateFn = fn;
        return *this;
    }
    template<typename T>
    StateBuilder& Update(std::function<void(T*, float)> fn) {
        _node.updateFn = [fn](AbstractState* s, float dt){ fn(static_cast<T*>(s), dt); };
        return *this;
    }

    StateBuilder& Event(const std::string& name, std::function<void(AbstractState*)> fn) {
        _node.events[name].fn = fn;
        return *this;
    }
    template<typename E>
    StateBuilder& Event(const std::string& name, std::function<void(AbstractState*, E*)> fn) {
        _node.events[name].fnWithData = [fn](AbstractState* s, std::shared_ptr<void> d){
            fn(s, static_cast<E*>(d.get()));
        };
        return *this;
    }
    template<typename S, typename E>
    StateBuilder& Event(const std::string& name, std::function<void(S*, E*)> fn) {
        _node.events[name].fnWithData = [fn](AbstractState* s, std::shared_ptr<void> d){
            fn(static_cast<S*>(s), static_cast<E*>(d.get()));
        };
        return *this;
    }

    StateBuilder& Condition(std::function<bool()> cond, std::function<void(AbstractState*)> action) {
        _node.conditions.push_back({cond, action});
        return *this;
    }
    template<typename T>
    StateBuilder& Condition(std::function<bool()> cond, std::function<void(T*)> action) {
        _node.conditions.push_back({cond, [action](AbstractState* s){ action(static_cast<T*>(s)); }});
        return *this;
    }

    StateNode&& Build() { return std::move(_node); }

    StateMachineBuilder& End();
};

// ---------------------------------------------------------------
// StateMachineBuilder
// ---------------------------------------------------------------
class StateMachineBuilder {
    std::unordered_map<std::string, StateNode> _states;
    std::vector<std::unique_ptr<StateBuilder>> _builders;

public:
    StateBuilder& State(const std::string& name) {
        _builders.emplace_back(std::make_unique<StateBuilder>(this, name));
        return *_builders.back();
    }

    template<typename T>
    StateBuilder& State(const std::string& name) {
        _builders.emplace_back(std::make_unique<StateBuilder>(this, name, std::make_shared<T>()));
        return *_builders.back();
    }

    void AddState(const std::string& name, StateNode&& node) {
        _states[name] = std::move(node);
    }

    std::shared_ptr<IState> Build() {
        for (auto& b : _builders) {
            auto node = b->Build();
            _states[node.name] = std::move(node);
        }
        return std::make_shared<StateMachine>(std::move(_states));
    }
};

inline StateMachineBuilder& StateBuilder::End() {
    _parent->AddState(_name, std::move(_node));
    return *_parent;
}

// CollisionEvent helper used in FSM
template<typename E>
struct CollisionEvent : public EventArgs {
    E* entity = nullptr;
};

} // namespace RSG

// Bring RSG types into scope  
using RSG::IState;
using RSG::AbstractState;
using RSG::StateMachineBuilder;
