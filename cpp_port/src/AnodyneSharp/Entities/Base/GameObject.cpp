#include "AnodyneSharp/Entities/Base/GameObject.hpp"
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include <cmath>

namespace AnodyneSharp::Entities {

Rectangle GameObject::Hitbox() const {
    return {(int)Position.X, (int)Position.Y, width, height};
}

void GameObject::PostUpdate() {
    lastPosition = Position;
    Vector2 oldVel = velocity;
    velocity.X += acceleration.X * GameTimes::DeltaTime();
    velocity.Y += acceleration.Y * GameTimes::DeltaTime();
    velocity.X = CalculateDrag(velocity.X, drag.X);
    velocity.Y = CalculateDrag(velocity.Y, drag.Y);
    float avgVX = (oldVel.X + velocity.X) / 2.f;
    float avgVY = (oldVel.Y + velocity.Y) / 2.f;
    Position.X += GameTimes::DeltaTime() * avgVX;
    Position.Y += GameTimes::DeltaTime() * avgVY;

    float oldAngVel = angularVelocity;
    angularVelocity += angularAcceleration * GameTimes::DeltaTime();
    rotation += (oldAngVel + (angularVelocity - oldAngVel) / 2.f) * GameTimes::DeltaTime();

    wasTouching = touching;
    touching = Touching::NONE;
}

float GameObject::CalculateDrag(float vel, float drag_val) {
    float d = drag_val * GameTimes::DeltaTime();
    if (vel < 0) return std::min(vel + d, 0.f);
    return std::max(vel - d, 0.f);
}

void GameObject::MoveTowards(Vector2 target, float speed) {
    Vector2 dir = {target.X - Position.X, target.Y - Position.Y};
    float len = std::sqrt(dir.X*dir.X + dir.Y*dir.Y);
    if (len > 0) { dir.X /= len; dir.Y /= len; }
    velocity.X = dir.X * speed;
    velocity.Y = dir.Y * speed;
}

bool GameObject::Separate(GameObject* o1, GameObject* o2) {
    bool x = SeparateX(o1, o2);
    bool y = SeparateY(o1, o2);
    return x || y;
}

bool GameObject::SeparateX(GameObject* o1, GameObject* o2) {
    if (o1->immovable && o2->immovable) return false;
    float overlap = 0;
    float d1 = o1->Position.X - o1->lastPosition.X;
    float d2 = o2->Position.X - o2->lastPosition.X;
    float d1a = std::abs(d1), d2a = std::abs(d2);
    float left1  = o1->Position.X - (d1>0?d1:0);
    float right1 = left1 + o1->width + d1a;
    float left2  = o2->Position.X - (d2>0?d2:0);
    float right2 = left2 + o2->width + d2a;
    if (right1 > left2 && left1 < right2 &&
        o1->lastPosition.Y + o1->height > o2->lastPosition.Y &&
        o1->lastPosition.Y < o2->lastPosition.Y + o2->height) {
        float maxOverlap = d1a + d2a + OVERLAP_BIAS;
        if (dynamic_cast<Entity*>(o2)) maxOverlap -= OVERLAP_BIAS/2;
        float overlapR = o1->Position.X + o1->width - o2->Position.X;
        float overlapL = o1->Position.X - o2->width  - o2->Position.X;
        if (std::abs(overlapR) < std::abs(overlapL)) {
            overlap = overlapR;
            if (overlap > maxOverlap ||
                (o1->allowCollisions & Touching::RIGHT) == Touching::NONE ||
                (o2->allowCollisions & Touching::LEFT)  == Touching::NONE) overlap = 0;
            else { o1->touching |= Touching::RIGHT; o2->touching |= Touching::LEFT; }
        } else {
            overlap = overlapL;
            if (-overlap > maxOverlap ||
                (o1->allowCollisions & Touching::LEFT)  == Touching::NONE ||
                (o2->allowCollisions & Touching::RIGHT) == Touching::NONE) overlap = 0;
            else { o1->touching |= Touching::LEFT; o2->touching |= Touching::RIGHT; }
        }
    }
    if (overlap != 0) {
        if (!o1->immovable && !o2->immovable) {
            overlap *= 0.5f;
            o1->Position.X -= overlap; o2->Position.X += overlap;
        } else if (!o1->immovable) {
            o1->Position.X -= overlap; o1->lastPosition.X = o1->Position.X;
        } else {
            o2->Position.X += overlap; o2->lastPosition.X = o2->Position.X;
        }
        return true;
    }
    return false;
}

bool GameObject::SeparateY(GameObject* o1, GameObject* o2) {
    if (o1->immovable && o2->immovable) return false;
    float overlap = 0;
    float d1 = o1->Position.Y - o1->lastPosition.Y;
    float d2 = o2->Position.Y - o2->lastPosition.Y;
    float d1a = std::abs(d1), d2a = std::abs(d2);
    float top1  = o1->Position.Y - (d1>0?d1:0);
    float bot1  = top1 + o1->height + d1a;
    float top2  = o2->Position.Y - (d2>0?d2:0);
    float bot2  = top2 + o2->height + d2a;
    if (bot1 > top2 && top1 < bot2 &&
        o1->lastPosition.X + o1->width > o2->lastPosition.X &&
        o1->lastPosition.X < o2->lastPosition.X + o2->width) {
        float maxOverlap = d1a + d2a + OVERLAP_BIAS;
        if (dynamic_cast<Entity*>(o2)) maxOverlap -= OVERLAP_BIAS/2;
        float overlapD = o1->Position.Y + o1->height - o2->Position.Y;
        float overlapU = o1->Position.Y - o2->height - o2->Position.Y;
        if (std::abs(overlapD) < std::abs(overlapU)) {
            overlap = overlapD;
            if (overlap > maxOverlap ||
                (o1->allowCollisions & Touching::DOWN) == Touching::NONE ||
                (o2->allowCollisions & Touching::UP)   == Touching::NONE) overlap = 0;
            else { o1->touching |= Touching::DOWN; o2->touching |= Touching::UP; }
        } else {
            overlap = overlapU;
            if (-overlap > maxOverlap ||
                (o1->allowCollisions & Touching::UP)   == Touching::NONE ||
                (o2->allowCollisions & Touching::DOWN) == Touching::NONE) overlap = 0;
            else { o1->touching |= Touching::UP; o2->touching |= Touching::DOWN; }
        }
    }
    if (overlap != 0) {
        if (!o1->immovable && !o2->immovable) {
            overlap *= 0.5f;
            o1->Position.Y -= overlap; o2->Position.Y += overlap;
        } else if (!o1->immovable) {
            o1->Position.Y -= overlap; o1->lastPosition.Y = o1->Position.Y;
        } else {
            o2->Position.Y += overlap; o2->lastPosition.Y = o2->Position.Y;
        }
        return true;
    }
    return false;
}

} // namespace AnodyneSharp::Entities
