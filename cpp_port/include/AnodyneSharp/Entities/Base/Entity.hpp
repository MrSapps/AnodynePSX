#pragma once
#include "AnodyneSharp/Entities/Base/GameObject.hpp"
#include "AnodyneSharp/Entities/Base/Rendering/SpriteRenderer.hpp"
#include "AnodyneSharp/GameEvents/Events.hpp"
#include "AnodyneSharp/Entities/Base/NamedEntity.hpp"

namespace AnodyneSharp::Entities {

enum class Facing { LEFT, RIGHT, UP, DOWN };

class Entity : public GameObject {
public:
    bool HasVisibleHitbox = false;

    Facing facing = Facing::DOWN;

    std::unique_ptr<ISpriteRenderer> sprite;

    std::string CurAnimName() const { return sprite ? sprite->CurAnimName() : ""; }
    bool AnimFinished() const        { return sprite ? sprite->AnimFinished() : true; }
    int  FrameIndex()  const         { return sprite ? sprite->FrameIndex() : 0; }
    int  Frame()       const         { return sprite ? sprite->Frame() : 0; }

    void set_layer(Drawing::DrawOrder value);
    ILayerType* layer_def_get() const;
    void        layer_def_set(ILayerType* value);

    Vector2 offset    = {0,0};
    float   opacity   = 1.f;
    float   scale     = 1.f;
    SpriteEffects _flip = SpriteEffects::None;
    float   y_push    = 0.f;

    bool _flickering = false;
    std::unique_ptr<class Shadow> shadow;

    bool MapInteraction = true;
    bool HoleAsWall     = false;

    Vector2 VisualCenter() const {
        return Position - offset + Vector2{(float)(sprite?sprite->Width():16),(float)(sprite?sprite->Height():16)}/2.f;
    }

    // Constructors matching C#
    Entity(Vector2 pos, int w = 0, int h = 0);
    Entity(Vector2 pos, Drawing::DrawOrder layer);
    Entity(Vector2 pos, const std::string& textureName, int fw, int fh, Drawing::DrawOrder layer);
    Entity(Vector2 pos, const std::string& textureName, int fw, int fh, ILayerType* layer);
    Entity(Vector2 pos, std::unique_ptr<ISpriteRenderer> spr);
    Entity(Vector2 pos, std::unique_ptr<ISpriteRenderer> spr, Drawing::DrawOrder layer);
    ~Entity();

    void PlayFacing(const std::string& AnimName);
    void Play(const std::string& AnimName, bool Force = false, std::optional<int> newFramerate = std::nullopt);
    virtual void Draw();
    void SetFrame(int frame);

    static Facing FacingFromTouching(Touching t);
    static Facing FlipFacing(Facing f);
    void FaceTowards(Vector2 target);
    static Vector2 FacingDirection(Facing f);

    void Flicker(float duration);

    void Update() override;
    void PostUpdate() override;

    virtual void Collided(Entity* other) {}
    virtual std::vector<Entity*> SubEntities() { return {}; }

    // Map interactions
    virtual void Fall(Vector2 fallPoint) { exists = false; }
    virtual void SlowTile()  {}
    virtual void Puddle()    {}
    virtual void Reflection() {}
    virtual void Ladder()    {}
    virtual void Conveyor(Touching direction) {}
    virtual void Grass()     {}
    virtual void OnEvent(GameEvents::GameEvent* e) {}

protected:
    virtual void AnimationChanged(const std::string& name) {}
    virtual void CenterOffset(bool updatePos = true);
    virtual bool SetTexture(const std::string& textureName, int fw, int fh,
                            bool ignoreChaos = false, bool allowFailure = false);

    void DrawImpl();

private:
    static constexpr float FlickerLength = 0.05f;
    float _flickerTimer = 0.f;
    float _flickerFreq  = 0.f;
    std::unique_ptr<ILayerType> _layerCache;

    void DoFlicker();
};

} // namespace AnodyneSharp::Entities

using AnodyneSharp::Entities::Entity;
using AnodyneSharp::Entities::Facing;
