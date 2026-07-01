#pragma once
#include "AnodyneSharp/Common.hpp"
#include "AnodyneSharp/FSM/CollisionEvent.hpp"
#include "RSG/RSG.hpp"
#include <vector>
#include <algorithm>

namespace AnodyneSharp::FSM {

class TimerState : public RSG::AbstractState {
public:
    struct Timer {
        float current = 0.f;
        float max = 0.f;
        std::string name;
    };

    std::vector<Timer> timers;
    float current = 0.f;

    void Reset() {
        timers.clear();
        current = 0.f;
    }

    void Advance(float time) { current += time; }

    void AddTimer(float time, const std::string& name) {
        Timer t;
        t.current = current + time;
        t.max = time;
        t.name = name;
        timers.push_back(t);
        std::sort(timers.begin(), timers.end(),
            [](const Timer& a, const Timer& b){ return a.current < b.current; });
    }

    void Update(float deltaTime) override {
        current += deltaTime;
        while (!timers.empty()) {
            auto& min = timers.front();
            if (min.current <= current) {
                std::string evName = min.name;
                float maxTime = min.max;
                timers.erase(timers.begin());
                TriggerEvent(evName);
                AddTimer(maxTime, evName);
            } else {
                break;
            }
        }
        RSG::AbstractState::Update(deltaTime);
    }
};

} // namespace AnodyneSharp::FSM

using AnodyneSharp::FSM::TimerState;
