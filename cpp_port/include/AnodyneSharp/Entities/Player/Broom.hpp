#pragma once
#include "AnodyneSharp/Entities/Base/Entity.hpp"

namespace AnodyneSharp::Entities {

class Player; // forward

class Broom : public Entity {
public:
    Entity* dust = nullptr;
    bool    IsDust = false;

    Broom(Player* player);

    void Attack();
    void UpdateBroomType();
    void Use(Facing direction);
    void Update() override;
    void Draw()   override;
    std::vector<Entity*> SubEntities() override;

private:
    Player* _player;
    void UpdatePos();
};

class Transformer : public Entity {
public:
    Transformer(Player* player);
    void Update()   override;
    void Draw()     override;
    void OnAction();
    void Reset();
    std::vector<Entity*> SubEntities() override;

private:
    Player*                  _player;
    std::unique_ptr<Entity>  _selector;
    std::unique_ptr<Entity>  _selectedTile;
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::Broom;
using AnodyneSharp::Entities::Transformer;
