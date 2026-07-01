#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp::GameEvents {

// Mirrors C# record GameEvent { }
struct GameEvent { virtual ~GameEvent() = default; };
struct StartScreenTransition : GameEvent {};
struct EndScreenTransition   : GameEvent {};
struct StartWarp             : GameEvent {};
struct EndWarp               : GameEvent {};
struct BroomUsed             : GameEvent {};
struct ChangeCardCount       : GameEvent { int Count = 0; ChangeCardCount(int c) : Count(c) {} };

} // namespace AnodyneSharp::GameEvents

using AnodyneSharp::GameEvents::GameEvent;
using AnodyneSharp::GameEvents::StartScreenTransition;
using AnodyneSharp::GameEvents::EndScreenTransition;
using AnodyneSharp::GameEvents::StartWarp;
using AnodyneSharp::GameEvents::EndWarp;
using AnodyneSharp::GameEvents::BroomUsed;
using AnodyneSharp::GameEvents::ChangeCardCount;
