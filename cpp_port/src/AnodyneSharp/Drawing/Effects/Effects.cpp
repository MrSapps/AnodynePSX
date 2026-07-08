#include "AnodyneSharp/Drawing/Effects/Effects.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"

namespace AnodyneSharp::Drawing::Effects {
    
    void FlashEffect::Update() 
    {
        float divAmount = 1.0f;
        if (_easing)
        {
            divAmount = 1.0f / GlobalState::settings.flash_easing;
            if (_alpha >= 1.0f)
            {
                if (_onFull) _onFull();
            }
        }
        else
        {
            divAmount = 1.0f / AnodyneSharp::GameTimes::DeltaTime();
        }

        if (_alpha > 0.f)
        {
            _alpha -= (1.f / _duration) * divAmount;
            if (_alpha < 0.f) 
            {
                _alpha = 0.f;
            }
        }
    }

    void FlashEffect::Flash(float duration, Color color, std::function<void()> onFull) {
        _alpha    = 1.f;
        _duration = duration > 0.f ? duration : 0.1f;
        _color    = color;
        _onFull   = onFull;
        if (GlobalState::settings.flash_easing == 0.0f)
        {
            _alpha = 1.0f;
            if (_onFull) _onFull();
        }
        else
        {
            _easing = true;
        }
    }
}
