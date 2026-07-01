#include "AnodyneSharp/MapData/Map.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/Utilities/MapUtilities.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "SDL3/SDL.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <filesystem>

// Minimal JSON string-value extractor (no external library)
static std::string jsonGetString(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos += search.size();
    auto end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}
static float jsonGetFloat(const std::string& json, const std::string& key, float def = 1.f) {
    std::string search = "\"" + key + "\":";
    auto pos = json.find(search);
    if (pos == std::string::npos) return def;
    pos += search.size();
    try { return std::stof(json.substr(pos)); } catch(...) { return def; }
}

namespace AnodyneSharp::MapData {

// ---- TileMap ----
TileMap::TileMap(const std::string& csv) {
    if (csv.empty()) return;
    std::istringstream ss(csv);
    std::string line;
    int w = 0, h = 0;
    std::vector<int> rows;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        std::string tok;
        int colCount = 0;
        while (std::getline(ls, tok, ',')) {
            try { rows.push_back(std::stoi(tok)); }
            catch(...) { rows.push_back(0); }
            ++colCount;
        }
        if (w == 0) w = colCount;
        ++h;
    }
    Width  = w;
    Height = h;
    _data  = std::move(rows);
}

int TileMap::GetTile(const Point& pos) const {
    if (pos.X < 0 || pos.X >= Width || pos.Y < 0 || pos.Y >= Height) return -1;
    int idx = pos.Y * Width + pos.X;
    if (idx < 0 || idx >= (int)_data.size()) return -1;
    return _data[idx];
}

void TileMap::ChangeTile(const Point& pos, int value) {
    if (pos.X < 0 || pos.X >= Width || pos.Y < 0 || pos.Y >= Height) return;
    _data[pos.Y * Width + pos.X] = value;
}

// ---- TileData loading helpers ----
static Touching ParseTouching(const std::string& s) {
    if (s == "NONE")  return Touching::NONE;
    if (s == "ANY")   return Touching::ANY;
    if (s == "LEFT")  return Touching::LEFT;
    if (s == "RIGHT") return Touching::RIGHT;
    if (s == "UP")    return Touching::UP;
    if (s == "DOWN")  return Touching::DOWN;
    return Touching::ANY;
}

static Tiles::CollisionEventType ParseEventType(const std::string& s) {
    using T = Tiles::CollisionEventType;
    if (s == "HOLE")       return T::HOLE;
    if (s == "SLOW")       return T::SLOW;
    if (s == "SPIKE")      return T::SPIKE;
    if (s == "LADDER")     return T::LADDER;
    if (s == "PUDDLE")     return T::PUDDLE;
    if (s == "REFLECTION") return T::REFLECTION;
    if (s == "GRASS")      return T::GRASS;
    if (s == "CONVEYOR")   return T::CONVEYOR;
    if (s == "THIN")       return T::THIN;
    return T::NONE;
}

// Apply .col file data to tile objects
static void LoadTileProperties(const std::string& mapName,
                                std::vector<std::unique_ptr<Tiles::Tile>>& tiles) {
    std::string path = ResourceManager::BaseDir + "/Content/Maps/" + mapName + "/TileData.col";
    SDL_Log("LoadTileProperties: path='%s' tilesSize=%d", path.c_str(), (int)tiles.size());
    std::ifstream f(path);
    if (!f.is_open()) { SDL_Log("LoadTileProperties: file not found, returning"); return; }
    SDL_Log("LoadTileProperties: file opened OK");

    // Strip UTF-8 BOM (EF BB BF) if present
    {
        char bom[3] = {};
        auto pos = f.tellg();
        if (f.read(bom, 3) && (unsigned char)bom[0]==0xEF && (unsigned char)bom[1]==0xBB && (unsigned char)bom[2]==0xBF) {
            SDL_Log("LoadTileProperties: stripped UTF-8 BOM");
        } else {
            f.seekg(pos);
        }
    }

    std::string line;
    int lineNum = 0;
    while (std::getline(f, line)) {
        ++lineNum;
        // trim \r
        if (!line.empty() && line.back() == '\r') line.pop_back();
        SDL_Log("LoadTileProperties: line %d = '%s'", lineNum, line.c_str());
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string indexPart, colPart, eventPart, dirPart;
        if (!std::getline(ss, indexPart, '\t') || !std::getline(ss, colPart, '\t')) continue;
        std::getline(ss, eventPart, '\t');
        std::getline(ss, dirPart, '\t');

        SDL_Log("LoadTileProperties: idx='%s' col='%s' evt='%s' dir='%s'",
            indexPart.c_str(), colPart.c_str(), eventPart.c_str(), dirPart.c_str());

        Touching allowed   = ParseTouching(colPart);
        Tiles::CollisionEventType evt = ParseEventType(eventPart);
        Touching dir       = dirPart.empty() ? Touching::ANY : ParseTouching(dirPart);

        int start = 0, end = 0;
        auto dash = indexPart.find('-');
        if (dash != std::string::npos) {
            start = std::stoi(indexPart.substr(0, dash));
            end   = std::stoi(indexPart.substr(dash + 1));
        } else {
            start = end = std::stoi(indexPart);
        }
        SDL_Log("LoadTileProperties: applying to tiles %d-%d", start, end);
        for (int i = start; i <= end && i < (int)tiles.size(); ++i) {
            SDL_Log("LoadTileProperties:   tile[%d] ptr=%p", i, (void*)tiles[i].get());
            tiles[i]->allowCollisions     = allowed;
            tiles[i]->collisionEventType  = evt;
            tiles[i]->direction           = dir;
        }
        SDL_Log("LoadTileProperties: line %d done", lineNum);
    }
    SDL_Log("LoadTileProperties: finished all lines");
}

// ---- Map ----
Map::Map(const std::string& name) : _mapName(name) {
    SDL_Log("Map::ctor(%s): start", name.c_str());
    // Load layer CSVs (BG, BG2, FG matching the MapLoader layer convention)
    auto loadLayer = [&](const char* layerFile) -> TileMap {
        std::string path = ResourceManager::BaseDir + "/Content/Maps/" + name + "/" + layerFile;
        std::ifstream f(path);
        if (!f.is_open()) return TileMap{""};
        std::string csv((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        return TileMap{csv};
    };

    _mapLayers[0] = loadLayer("BG.csv");  // BG
    _mapLayers[1] = loadLayer("BG2.csv"); // BG2
    _mapLayers[2] = loadLayer("FG.csv");  // FG

    WidthInTiles  = _mapLayers[0].Width;
    HeightInTiles = _mapLayers[0].Height;

    // Create tile objects — 512 slots is sufficient for all maps; tile 0 = no collision/invisible
    constexpr int TILE_COUNT = 512;
    _tileObjects.resize(TILE_COUNT);
    for (int i = 0; i < TILE_COUNT; ++i) {
        bool vis = (i >= 1);
        Touching coll = vis ? Touching::ANY : Touching::NONE;
        _tileObjects[i] = std::make_unique<Tiles::Tile>(16, 16, vis, coll);
    }
    SDL_Log("Map::ctor(%s): calling LoadTileProperties", name.c_str());
    LoadTileProperties(name, _tileObjects);
    SDL_Log("Map::ctor(%s): LoadTileProperties done", name.c_str());

    // Load animated tiles from Content/Maps/{name}/TileAnims/*.dat
    // Format per line: tileIndex\tframerate\tframe1,frame2,...
    // Filename stem = texture name (e.g. "fields_anims")
    {
        namespace fs = std::filesystem;
        std::string animsDir = ResourceManager::BaseDir + "/Content/Maps/" + name + "/TileAnims";
        std::error_code ec;
        for (auto& entry : fs::directory_iterator(animsDir, ec)) {
            if (entry.path().extension() != ".dat") continue;
            std::string texName = entry.path().stem().string();
            std::ifstream af(entry.path());
            std::string aline;
            while (std::getline(af, aline)) {
                if (!aline.empty() && aline.back() == '\r') aline.pop_back();
                if (aline.empty()) continue;
                std::istringstream as(aline);
                std::string idxTok, fpsTok, framesTok;
                if (!std::getline(as, idxTok, '\t') ||
                    !std::getline(as, fpsTok, '\t') ||
                    !std::getline(as, framesTok, '\t')) continue;
                int tileIdx = std::stoi(idxTok);
                float fps   = std::stof(fpsTok);
                std::vector<int> frames;
                std::istringstream fs2(framesTok);
                std::string ft;
                while (std::getline(fs2, ft, ',')) {
                    if (!ft.empty()) frames.push_back(std::stoi(ft));
                }
                if (!frames.empty())
                    _animatedTiles.emplace(tileIdx,
                        AnimatedTile{frames, fps, texName});
            }
        }
    }

    // Load the tileset spritesheet — name follows the pattern: lowercase(mapname)_tilemap
    std::string tilesetName = name;
    std::transform(tilesetName.begin(), tilesetName.end(), tilesetName.begin(), ::tolower);
    tilesetName += "_tilemap";
    SDL_Log("Map::ctor(%s): loading tileset '%s'", name.c_str(), tilesetName.c_str());
    auto* tilesetTex = ResourceManager::GetTexHandle(tilesetName, true);
    SDL_Log("Map::ctor(%s): tileset tex=%p", name.c_str(), (void*)tilesetTex);
    _tiles = Spritesheet(tilesetTex, 16, 16);
    SDL_Log("Map::ctor(%s): done", name.c_str());
}

void Map::Draw(const Rectangle& bounds) {
    DrawLayer(bounds, Layer::BG,  Drawing::DrawOrder::MAP_BG);
    DrawLayer(bounds, Layer::BG2, Drawing::DrawOrder::MAP_BG2, true);
    DrawLayer(bounds, Layer::FG,  Drawing::DrawOrder::MAP_FG, true);
}

void Map::Update() {
    for (auto& [idx, anim] : _animatedTiles)
        anim.UpdateAnimation();
}

void Map::Collide(Entities::Entity* e) {
    if (!e || !e->exists) return;
    Rectangle hb = e->Hitbox();
    // Clamp to tile grid
    Point tl = ToMapLoc(Vector2{(float)hb.X,           (float)hb.Y           });
    Point br = ToMapLoc(Vector2{(float)(hb.X+hb.Width), (float)(hb.Y+hb.Height)});
    for (int y = tl.Y; y <= br.Y; ++y) {
        for (int x = tl.X; x <= br.X; ++x) {
            Point p{x, y};
            int bg = _mapLayers[0].GetTile(p);
            if (bg >= 0 && bg < (int)_tileObjects.size())
                CollideTile(p, _tileObjects[bg].get(), e);
            int bg2 = _mapLayers[1].GetTile(p);
            if (bg2 > 0 && bg2 < (int)_tileObjects.size())
                CollideTile(p, _tileObjects[bg2].get(), e);
        }
    }
}

void Map::ReloadSettings(const Vector2& player_pos, bool graphics_only,
                          Entities::Player* p, bool screen_transition) {
    // Load Settings.json for this map
    std::string settingsPath = ResourceManager::BaseDir
        + "/Content/Maps/" + _mapName + "/Settings.json";
    std::ifstream f(settingsPath);
    if (!f.is_open()) return;
    std::string json((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    std::string music     = jsonGetString(json, "Music");
    std::string ambience  = jsonGetString(json, "Ambience");
    std::string darkness  = jsonGetString(json, "Darkness");
    float musicVol        = jsonGetFloat(json, "MusicVolume",   1.f);
    float ambienceVol     = jsonGetFloat(json, "AmbienceVolume",1.f);
    float darknessAlpha   = jsonGetFloat(json, "DarknessAlpha", darkness.empty() ? 0.f : 1.f);

    if (!graphics_only) {
        if (!_ignoreMusic) {
            Sounds::SoundManager::PlaySong(Registry::GlobalState::InDeathRoom ? "" : music, musicVol);
            Sounds::SoundManager::PlayAmbience(Registry::GlobalState::InDeathRoom ? "" : ambience, ambienceVol);
        }
        _ignoreMusic = false;
    }

    // Darkness overlay
    Registry::GlobalState::darkness.SetTex(darkness);
    float targetAlpha = Registry::GlobalState::InDeathRoom ? 0.8f : darknessAlpha;
    if (!screen_transition) {
        Registry::GlobalState::darkness.ForceAlpha(targetAlpha);
    } else {
        Registry::GlobalState::darkness.TargetAlpha(targetAlpha);
    }

    // FG / Extra blend overlays
    Registry::GlobalState::fgBlend.SetTex(jsonGetString(json, "FG_Blend"));
    Registry::GlobalState::extraBlend.SetTex(jsonGetString(json, "ExtraBlend"));

    // Player light (skip — requires full PlayerLight type; handled via EventsAndLights.hpp)
    (void)p;

    // TileReplacement: "ReplaceTiles": "20-21;30-31"
    auto jsonGetStringOpt = [&](const std::string& k) -> std::string {
        return jsonGetString(json, k);
    };
    std::string replaceTiles = jsonGetStringOpt("ReplaceTiles");
    _replacements.clear();
    if (!replaceTiles.empty()) {
        std::istringstream rs(replaceTiles);
        std::string pair;
        while (std::getline(rs, pair, ';')) {
            auto dash = pair.find('-');
            if (dash == std::string::npos) continue;
            try {
                int from = std::stoi(pair.substr(0, dash));
                int to   = std::stoi(pair.substr(dash + 1));
                _replacements[from] = to;
            } catch (...) {}
        }
    }
}

void Map::ReloadSettings(Entities::Player* p, bool graphics_only) {
    ReloadSettings(Vector2{0,0}, graphics_only, p, false);
}

void Map::OnTransitionStart() {
    for (auto& layer : _mapLayers) layer.OnTransitionStart();
}

void Map::OnTransitionEnd() {
    for (auto& layer : _mapLayers) layer.OnTransitionEnd();
}

int Map::GetTile(Layer layer, const Point& pos) {
    int raw = _mapLayers[(int)layer].GetTile(pos);
    // Apply tile replacements only on BG layer
    if (layer == Layer::BG && raw >= 0) {
        auto it = _replacements.find(raw);
        if (it != _replacements.end()) return it->second;
    }
    return raw;
}

void Map::ChangeTile(Layer layer, const Point& pos, int newVal) {
    _mapLayers[(int)layer].ChangeTile(pos, newVal);
}

Point Map::ToMapLoc(const Vector2& pos) {
    Vector2 p = pos + offset;
    return {(int)(p.X / Registry::GameConstants::TILE_WIDTH),
            (int)(p.Y / Registry::GameConstants::TILE_WIDTH)};
}

Touching Map::GetCollisionData(const Vector2& pos) {
    Point p = ToMapLoc(pos);
    int bg = _mapLayers[0].GetTile(p);
    if (bg < 0 || bg >= (int)_tileObjects.size()) return Touching::NONE;
    Touching ret = _tileObjects[bg]->allowCollisions;
    int bg2 = _mapLayers[1].GetTile(p);
    if (bg2 > 0 && bg2 < (int)_tileObjects.size())
        ret = (Touching)((int)ret | (int)_tileObjects[bg2]->allowCollisions);
    return ret;
}

Vector2 Map::TileToWorld(const Point& p) {
    return Vector2{(float)(p.X * Registry::GameConstants::TILE_WIDTH),
                   (float)(p.Y * Registry::GameConstants::TILE_WIDTH)} - offset;
}

void Map::DrawLayer(const Rectangle& bounds, Layer map,
                    Drawing::DrawOrder layer, bool ignoreEmpty) {
    if (!_tiles.GetTex()) return;
    float z = Drawing::DrawingUtilities::GetDrawingZ(layer, 0);
    Point tl = ToMapLoc(Vector2{(float)bounds.X,               (float)bounds.Y});
    Point br = ToMapLoc(Vector2{(float)(bounds.X + bounds.Width), (float)(bounds.Y + bounds.Height)});
    bool isBG = (map == Layer::BG);
    for (int y = tl.Y - 1; y < br.Y + 1; ++y) {
        for (int x = tl.X - 1; x < br.X + 1; ++x) {
            int tile = GetTile(map, Point{x, y});
            if (tile < 0 || tile >= (int)_tileObjects.size()) continue;
            if (!_tileObjects[tile]->visible) continue;
            if (ignoreEmpty && tile == 0) continue;

            Vector2 loc = TileToWorld(Point{x, y});
            Rectangle dst{(int)loc.X, (int)loc.Y, _tiles.Width, _tiles.Height};

            // For BG layer, check for animated tile override
            if (isBG) {
                auto it = _animatedTiles.find(tile);
                if (it != _animatedTiles.end()) {
                    Rectangle src = it->second.spriteRect;
                    Drawing::SpriteDrawer::DrawSprite(it->second.sprite.GetTex(), dst, &src, nullptr, 0.f, SpriteEffects::None, z);
                    continue;
                }
            }

            Rectangle src = _tiles.GetRect(tile);
            Drawing::SpriteDrawer::DrawSprite(_tiles.GetTex(), dst, &src, nullptr, 0.f, SpriteEffects::None, z);
        }
    }
}

void Map::CollideTile(const Point& tilePos, Tiles::Tile* t, Entities::Entity* ent) {
    using namespace Tiles;
    if (!t) return;

    t->lastPosition = t->Position = TileToWorld(tilePos);

    // Treat holes as walls for entities with HoleAsWall=true
    if (ent->HoleAsWall && t->collisionEventType == CollisionEventType::HOLE) {
        Touching saved = t->allowCollisions;
        t->allowCollisions = Touching::ANY;
        Entities::GameObject::Separate(ent, t);
        t->allowCollisions = saved;
        return;
    }

    bool separated = (t->allowCollisions == Touching::NONE) ||
                     Entities::GameObject::Separate(ent, t);

    if (separated && t->collisionEventType != CollisionEventType::NONE && ent->MapInteraction) {
        switch (t->collisionEventType) {
        case CollisionEventType::CONVEYOR:
            if (t->Hitbox().Contains(ent->Center()))
                ent->Conveyor(t->direction);
            break;

        case CollisionEventType::THIN: {
            // Create a thin barrier on the specified edge
            Entities::GameObject collider{0, 0};
            Vector2 tp = t->Position;
            if (t->direction == Touching::DOWN) {
                collider = Entities::GameObject{t->width, 3};
                collider.Position = tp + Vector2{0.f, 13.f};
            } else if (t->direction == Touching::UP) {
                collider = Entities::GameObject{t->width, 3};
                collider.Position = tp;
            } else if (t->direction == Touching::LEFT) {
                collider = Entities::GameObject{3, t->height};
                collider.Position = tp;
            } else if (t->direction == Touching::RIGHT) {
                collider = Entities::GameObject{3, t->height};
                collider.Position = tp + Vector2{13.f, 0.f};
            }
            collider.immovable = true;
            Entities::GameObject::Separate(ent, &collider);
            break;
        }
        case CollisionEventType::HOLE: {
            Rectangle actualHitbox = t->Hitbox();
            actualHitbox.Y += 5; actualHitbox.Height = 4;
            actualHitbox.X += 5; actualHitbox.Width  = 6;
            if (actualHitbox.Intersects(ent->Hitbox()))
                ent->Fall(t->Position);
            break;
        }
        case CollisionEventType::SLOW:
            ent->SlowTile();
            break;

        case CollisionEventType::SPIKE: {
            auto* player = dynamic_cast<Entities::Player*>(ent);
            if (player && player->state != PlayerState::AIR) {
                Rectangle actualHitbox = t->Hitbox();
                actualHitbox.Y += 6; actualHitbox.Height = 6;
                actualHitbox.X += 6; actualHitbox.Width  = 5;
                if (actualHitbox.Intersects(ent->Hitbox()))
                    player->ReceiveDamage(1, SpikeDamageDealer, false);
            }
            break;
        }
        case CollisionEventType::LADDER:
            ent->Ladder();
            break;

        case CollisionEventType::PUDDLE:
            if (t->Hitbox().Contains(ent->Center()))
                ent->Puddle();
            break;

        case CollisionEventType::REFLECTION:
            if (t->Hitbox().Contains(ent->Center())) {
                ent->Puddle();
                ent->Reflection();
            }
            break;

        case CollisionEventType::GRASS:
            if (t->Hitbox().Contains(ent->Center()))
                ent->Grass();
            break;

        default: break;
        }
    }
}

// ---- MapLoader ----
TileMap MapLoader::GetMapLayer(const std::string& mapName, int layer) {
    std::string path = ResourceManager::BaseDir + "/Content/Maps/" + mapName + "/" + std::to_string(layer) + ".csv";
    std::ifstream f(path);
    if (!f.is_open()) return TileMap{"0,0"};
    std::string csv((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return TileMap{csv};
}

TileMap MapLoader::GetMinimap(const std::string& mapName) {
    std::string path = ResourceManager::BaseDir + "/Content/MiniMaps/Minimap_" + mapName + ".csv";
    std::ifstream f(path);
    if (!f.is_open()) return TileMap{};
    std::string csv((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return TileMap{csv};
}

// ---- Minimap ----
Minimap::Minimap(const std::string& name) : mapName(name) {
    tiles = MapLoader::GetMinimap(name);
    interest.assign(tiles.Width * tiles.Height, 0);
}

void Minimap::Update() {
    int x = Registry::GlobalState::CURRENT_GRID_X;
    int y = Registry::GlobalState::CURRENT_GRID_Y;
    if (x >= 0 && x < tiles.Width && y >= 0 && y < tiles.Height)
        interest[x + y * tiles.Width] = std::max(1, interest[x + y * tiles.Width]);
}

void Minimap::AddInterest() {
    int loc = Registry::GlobalState::CURRENT_GRID_X + Registry::GlobalState::CURRENT_GRID_Y * tiles.Width;
    if (loc >= 0 && loc < (int)interest.size()) ++interest[loc];
}

void Minimap::RemoveInterest() {
    int loc = Registry::GlobalState::CURRENT_GRID_X + Registry::GlobalState::CURRENT_GRID_Y * tiles.Width;
    if (loc >= 0 && loc < (int)interest.size()) --interest[loc];
}

void Minimap::Draw(const Spritesheet& sprites, Vector2 topleft,
                   const Rectangle* bounds, bool drawPlayer) {
    if (!sprites.GetTex()) return;
    Rectangle b = bounds ? *bounds : Rectangle{0, 0, tiles.Width, tiles.Height};
    float zMini   = Drawing::DrawingUtilities::GetDrawingZ(DrawOrder::MINIMAP);
    float zChest  = Drawing::DrawingUtilities::GetDrawingZ(DrawOrder::MINIMAP_CHEST);
    float zPlayer = Drawing::DrawingUtilities::GetDrawingZ(DrawOrder::MINIMAP_PLAYER);

    for (int y = b.Y; y < b.Y + b.Height; ++y) {
        for (int x = b.X; x < b.X + b.Width; ++x) {
            int idx = x + y * tiles.Width;
            if (idx < 0 || idx >= (int)interest.size()) continue;
            int vis = interest[idx];
            if (vis > 0) {
                Rectangle src = sprites.GetRect(tiles.GetTile(Point{x, y}));
                Vector2 pos{topleft.X + x * sprites.Width, topleft.Y + y * sprites.Height};
                Drawing::SpriteDrawer::DrawSprite(sprites.GetTex(), pos, &src, nullptr, 0.f, 1.f, zMini);
            }
            if (vis > 1) {
                Rectangle src = sprites.GetRect(ChestIndicator);
                Vector2 pos{topleft.X + x * sprites.Width, topleft.Y + y * sprites.Height};
                Drawing::SpriteDrawer::DrawSprite(sprites.GetTex(), pos, &src, nullptr, 0.f, 1.f, zChest);
            }
        }
    }
    if (drawPlayer) {
        int px = Registry::GlobalState::CURRENT_GRID_X;
        int py = Registry::GlobalState::CURRENT_GRID_Y;
        bool inBounds = (px >= b.X && px < b.X + b.Width && py >= b.Y && py < b.Y + b.Height);
        if (inBounds) {
            Rectangle src = sprites.GetRect(PlayerIndicator);
            Vector2 pos{topleft.X + px * (float)sprites.Width, topleft.Y + py * (float)sprites.Height};
            Drawing::SpriteDrawer::DrawSprite(sprites.GetTex(), pos, &src, nullptr, 0.f, 1.f, zPlayer);
        }
    }
}

} // namespace AnodyneSharp::MapData

// ---- SwapperControl ----
namespace AnodyneSharp::MapData {

SwapperControl::SwapperControl(const std::string& mapName) {
    std::string path = Resources::ResourceManager::BaseDir
        + "/Content/Maps/" + mapName + "/Swapper.dat";
    std::ifstream f(path);
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string stateTok, xTok, yTok, wTok, hTok;
        if (!std::getline(ss, stateTok, '\t') ||
            !std::getline(ss, xTok,    '\t') ||
            !std::getline(ss, yTok,    '\t') ||
            !std::getline(ss, wTok,    '\t') ||
            !std::getline(ss, hTok,    '\t')) continue;
        State s = State::NONE;
        if      (stateTok == "Allow")             s = State::ON;
        else if (stateTok == "Disallow")          s = State::OFF;
        else if (stateTok == "DisallowSilently")  s = State::SILENT_OFF;
        // Default → NONE (skip)
        if (s == State::NONE) continue;
        try {
            _regions.push_back({s, Rectangle{std::stoi(xTok), std::stoi(yTok),
                                              std::stoi(wTok), std::stoi(hTok)}});
        } catch (...) {}
    }
}

SwapperControl::State SwapperControl::CheckCoord(const Vector2& pos) const {
    for (auto& r : _regions) {
        if (r.area.Contains(Point{(int)pos.X, (int)pos.Y}))
            return r.state;
    }
    return State::NONE;
}

} // namespace AnodyneSharp::MapData
