#pragma once
#include "AnodyneSharp/Common.hpp"
#include "RSG/RSG.hpp"

namespace AnodyneSharp::FSM {

template<typename E>
struct CollisionEvent : public RSG::EventArgs {
    E* entity = nullptr;
};

} // namespace AnodyneSharp::FSM

using AnodyneSharp::FSM::CollisionEvent;
