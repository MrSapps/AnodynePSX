#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"

namespace AnodyneSharp::Entities {

class Parabola_Thing {
public:
    Parabola_Thing(Entity* entity, float height, float period)
        : _entity(entity), _height(height), _period(period), _start(entity->offset.Y) {}

    bool Tick() {
        _t += GameTimes::DeltaTime();
        if (_t > _period) {
            _entity->offset.Y = _start;
            if (_entity->shadow) _entity->shadow->UpdateShadow(0.f);
            return true;
        }
        float half = _period / 2.f;
        _entity->offset.Y = _start + _t * (_period - _t) / (half*half) * _height;
        if (_entity->shadow) {
            _entity->shadow->UpdateShadow(std::abs(0.5f - Progress()) * 2.f);
        }
        return false;
    }

    float Progress() const { return std::min(_t / _period, 1.f); }
    void  ResetTime() { _t = 0.f; }

private:
    float   _t = 0.f;
    float   _height;
    float   _period;
    float   _start;
    Entity* _entity;
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::Parabola_Thing;
