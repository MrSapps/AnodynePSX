#pragma once
#include "AnodyneSharp/Common.hpp"
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/Drawing/Spritesheet/Spritesheet.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include <map>

namespace AnodyneSharp::Entities { class Player; } // forward

namespace AnodyneSharp::MapData {

enum class Layer { BG, BG2, FG };

// AnimatedTile — drives frame cycling for tile animations
struct AnimatedTile {
    Spritesheet     sprite;
    Rectangle       spriteRect{};
    std::vector<int> frames;
    float           framerate = 3.f;
    float           _timer    = 0.f;
    int             _frameIdx = 0;

    AnimatedTile() = default;
    AnimatedTile(const std::vector<int>& f, float fps, const std::string& texName)
        : frames(f), framerate(fps)
    {
        sprite = Spritesheet(Resources::ResourceManager::GetTexHandle(texName, true), 16, 16);
        if (!frames.empty()) spriteRect = sprite.GetRect(frames[0]);
    }

    void UpdateAnimation() {
        if (frames.empty()) return;
        _timer += GameTimes::DeltaTime() * framerate;
        if (_timer >= 1.f) {
            _timer -= 1.f;
            _frameIdx = (_frameIdx + 1) % (int)frames.size();
            spriteRect = sprite.GetRect(frames[_frameIdx]);
        }
    }
};

struct IPublicMap {
    virtual ~IPublicMap() = default;
    virtual int      GetTile(Layer layer, const Point& pos) = 0;
    virtual void     ChangeTile(Layer layer, const Point& pos, int newVal) = 0;
    virtual Point    ToMapLoc(const Vector2& pos) = 0;
    virtual Touching GetCollisionData(const Vector2& pos) = 0;
};

class TileMap {
public:
    int Width  = 0;
    int Height = 0;

    TileMap() = default;
    explicit TileMap(const std::string& csv);

    int  GetTile(const Point& pos) const;
    void ChangeTile(const Point& pos, int value);
    void OnTransitionStart() {}
    void OnTransitionEnd()   {}

private:
    std::vector<int> _data;
};

namespace Tiles {

enum class CollisionEventType {
    NONE, HOLE, SLOW, SPIKE, LADDER, PUDDLE, REFLECTION, GRASS, CONVEYOR, THIN
};

struct Tile : public Entities::GameObject {
    bool visible      = true;
    CollisionEventType collisionEventType = CollisionEventType::NONE;
    Touching direction = Touching::NONE;

    Tile(int w, int h, bool visible, Touching allowColl)
        : GameObject(w, h), visible(visible) {
        this->allowCollisions = allowColl;
    }
};

} // namespace Tiles

class Map : public IPublicMap {
public:
    static constexpr const char* SpikeDamageDealer = "Ground spikes";

    int WidthInTiles  = 0;
    int HeightInTiles = 0;
    Vector2 offset = {0,0};

    Map(const std::string& name);

    void Draw(const Rectangle& bounds);
    void Update();
    void Collide(Entities::Entity* e);
    void ReloadSettings(const Vector2& player_pos, bool graphics_only = false,
                        Entities::Player* p = nullptr, bool screen_transition = true);
    void ReloadSettings(Entities::Player* p, bool graphics_only = false);
    void OnTransitionStart();
    void OnTransitionEnd();
    void IgnoreMusicNextUpdate() { _ignoreMusic = true; }

    int      GetTile(Layer layer, const Point& pos) override;
    void     ChangeTile(Layer layer, const Point& pos, int newVal) override;
    Point    ToMapLoc(const Vector2& pos) override;
    Touching GetCollisionData(const Vector2& pos) override;

    Vector2  TileToWorld(const Point& p);
    class SwapperControl* swapper_ref = nullptr;

private:
    std::array<TileMap, 3> _mapLayers;
    Drawing::Spritesheet::Spritesheet _tiles;
    std::vector<std::unique_ptr<Tiles::Tile>> _tileObjects;
    std::map<int, AnimatedTile> _animatedTiles;    // tileIndex → animated tile
    std::unordered_map<int, int> _replacements;    // tileIndex → replacement

    std::string _mapName;
    bool _ignoreMusic = false;

    void DrawLayer(const Rectangle& bounds, Layer map, Drawing::DrawOrder layer, bool ignoreEmpty=false);
    void CollideTile(const Point& tilePos, Tiles::Tile* t, Entities::Entity* ent);
};

class MapLoader {
public:
    static TileMap GetMapLayer(const std::string& mapName, int layer = 1);
    static TileMap GetMinimap(const std::string& mapName);
};

class Minimap {
public:
    TileMap tiles;
    std::string mapName;
    std::vector<int> interest;

    static constexpr int PlayerIndicator = 27;
    static constexpr int ChestIndicator  = 18;

    Minimap() = default;
    Minimap(const std::string& name);

    void Update();
    void Draw(const Spritesheet& sprites, Vector2 topleft, const Rectangle* bounds = nullptr, bool drawPlayer = true);
    void AddInterest();
    void RemoveInterest();
};

class MinimapTracker {
public:
    std::unordered_map<std::string, std::vector<int>> interest;
    std::unordered_map<std::string, std::unique_ptr<Minimap>> _minimaps;

    Minimap* GetMinimap(const std::string& name) {
        auto it = _minimaps.find(name);
        if (it == _minimaps.end()) {
            _minimaps[name] = std::make_unique<Minimap>(name);
        }
        return _minimaps[name].get();
    }
};

class SwapperControl {
public:
    enum class State { NONE, ON, OFF, SILENT_OFF };

    SwapperControl() = default;
    SwapperControl(const std::string& mapName);

    State CheckCoord(const Vector2& pos) const;

private:
    struct Region { State state; Rectangle area; };
    std::vector<Region> _regions;
};

} // namespace AnodyneSharp::MapData

using AnodyneSharp::MapData::Map;
using AnodyneSharp::MapData::TileMap;
// Note: AnodyneSharp::MapData::Layer is NOT imported to global scope
// to avoid collision with AnodyneSharp::Entities::Base::Rendering::Layer
using AnodyneSharp::MapData::Minimap;
using AnodyneSharp::MapData::MinimapTracker;
using AnodyneSharp::MapData::SwapperControl;
using AnodyneSharp::MapData::MapLoader;
using AnodyneSharp::MapData::IPublicMap;
