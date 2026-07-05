#pragma once
#include "AnodyneSharp/Common.hpp"
#include "AnodyneSharp/GameTimes.hpp"

namespace AnodyneSharp::Entities {

enum class Touching {
    NONE  = 0,
    LEFT  = 1,
    RIGHT = 2,
    UP    = 4,
    DOWN  = 8,
    ANY   = LEFT | RIGHT | UP | DOWN
};

inline Touching operator|(Touching a, Touching b)  { return Touching((int)a|(int)b); }
inline Touching operator&(Touching a, Touching b)  { return Touching((int)a&(int)b); }
inline Touching& operator|=(Touching& a, Touching b){ a = a|b; return a; }
inline bool     operator!(Touching a)               { return a == Touching::NONE; }

class Entity; // forward

class GameObject {
public:
    static constexpr float OVERLAP_BIAS = 4.f;

    Rectangle Hitbox() const;
    Vector2   Center() const { return Position + Vector2{(float)width,(float)height}/2.f; }

    bool Solid() const { return allowCollisions != Touching::NONE; }
    void SetSolid(bool v) { allowCollisions = v ? Touching::ANY : Touching::NONE; }

    Vector2  Position     = {0,0};
    Vector2  lastPosition = {0,0};
    Vector2  velocity     = {0,0};
    Vector2  acceleration = {0,0};
    Vector2  drag         = {0,0};

    bool     immovable    = false;
    float    rotation     = 0.f;
    float    angularVelocity     = 0.f;
    float    angularAcceleration = 0.f;

    bool     exists       = true;
    
    int      width  = 0;
    int      height = 0;
    bool     visible      = true;
    Touching touching        = Touching::NONE;
    Touching wasTouching     = Touching::NONE;
    Touching allowCollisions = Touching::ANY;

    GameObject() = default;
    GameObject(Vector2 pos) : Position(pos), lastPosition(pos), visible(true) {}
    GameObject(Vector2 pos, int w, int h) : Position(pos), lastPosition(pos), width(w), height(h), visible(true) {}
    GameObject(int w, int h) : width(w), height(h), visible(true) {}

    virtual void Update() {}
    virtual void PostUpdate();

    static bool Separate(GameObject* o1, GameObject* o2);
    static bool SeparateX(GameObject* o1, GameObject* o2);
    static bool SeparateY(GameObject* o1, GameObject* o2);

protected:
    void MoveTowards(Vector2 target, float speed);

private:
    float CalculateDrag(float velocity, float drag);
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::Touching;
using AnodyneSharp::Entities::GameObject;
