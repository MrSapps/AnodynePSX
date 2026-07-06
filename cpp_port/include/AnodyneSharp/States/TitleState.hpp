#pragma once

#include "AnodyneSharp/States/Base/State.hpp"
#include "AnodyneSharp/Drawing/ScrollingTex.hpp"
#include "AnodyneSharp/UI/UITypes.hpp"
#include "RSG/RSG.hpp"
#include <vector>
#include <memory>


namespace AnodyneSharp::FSM
{
    class TimerState : public RSG::AbstractState
    {
    public:

    private:
        class Timer
        {
        public:
            float current = 0.0f;
            float max = 0.0f;
            std::string name;
        };

        std::vector<Timer> timers;
        float current = 0.0f;

        void Reset()
        {
            timers.clear();
        }

        void Advance(float time)
        {
            current += time;
        }

    protected:
        void AddTimer(float time, const std::string& name)
        {
            Timer t;
            t.current = current + time;
            t.max = time;
            t.name = name;

            timers.push_back(t);

            std::sort(timers.begin(), timers.end(),
                [](const Timer& a, const Timer& b)
                {
                    return a.current < b.current;
                });
        }

        void Update(float deltaTime) override
        {
            current += deltaTime;

            if (!timers.empty())
            {
                // always check the earliest timer
                while (!timers.empty() && timers.front().current <= current)
                {
                    Timer min = timers.front();
                    TriggerEvent(min.name);

                    // remove first
                    timers.erase(timers.begin());

                    // re-add with new scheduled time
                    AddTimer(min.max, min.name);
                }
            }

            AbstractState::Update(deltaTime);
        }

    };
}

namespace AnodyneSharp::States {

// ---- TitleState ----
class TitleState : public State {
public:
    TitleState();
    void Create()  override;
    void Update()  override;
    void Draw()    override;
    void DrawUI()  override;

private:
    bool AnyKeyPressed() const;

    Drawing::ScrollingTex _background;

    std::shared_ptr<RSG::IState> _state;

    std::unique_ptr<UIEntity> mNexusImage;
    std::unique_ptr<UIEntity> mDoorGlow;
    std::unique_ptr<UIEntity> mDoorSpin1;
    std::unique_ptr<UIEntity> mDoorSpin2;

    std::unique_ptr<UIEntity> mTitleTex;
    std::unique_ptr<UIEntity> mTitleOverlay;    
    std::unique_ptr<UIEntity> mPressEnterTex;

    std::unique_ptr<UIEntity> mSubtitle;
    std::unique_ptr<UIEntity> mSubtitleOverlay;

    std::string mCredits[3];
    UILabel* mCreditLabels[3] = {nullptr, nullptr, nullptr};
    std::vector<std::pair<int, int>> notVisibleYet;

    bool _secondNames = false;

    bool  _pressEnterVisible = false;
    float _blinkTimer        = 1.f;
    bool  _pixelating        = false;
};
}
