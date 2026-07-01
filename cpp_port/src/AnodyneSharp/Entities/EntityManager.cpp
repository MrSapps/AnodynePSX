#include "AnodyneSharp/Entities/EntityManager.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Entities/Player/Broom.hpp"
#include "AnodyneSharp/Entities/Enemy/AllEnemies.hpp"
#include "AnodyneSharp/Entities/Gadget/AllGadgets.hpp"
#include "AnodyneSharp/Entities/Interactive/AllInteractive.hpp"
#include "AnodyneSharp/Entities/Decorations/Decorations.hpp"
#include "AnodyneSharp/Entities/EventsAndLights.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include <fstream>
#include <sstream>
#include <regex>

namespace AnodyneSharp::Entities {

std::unordered_map<Guid, EntityState, Guid::Hash> EntityManager::State;
std::unordered_map<std::string, std::vector<std::unique_ptr<EntityPreset>>> EntityManager::_entities;
std::unordered_map<int, DoorPair*> EntityManager::_doorPairs;
std::unordered_map<int, std::vector<EntityPreset*>> EntityManager::_linkGroups;

// Helper macro: register entity XML name to C++ constructor
#define REG(xmlName, ClassName) \
    fac[xmlName] = [](EntityPreset* p, Player* pl) { return std::make_unique<ClassName>(p, pl); }

static void RegisterAllEntities() {
    auto& fac = EntityPreset::Factories();
    // --- Enemies ---
    REG("Annoyer",       Annoyer);
    REG("Chaser",        Chaser);
    REG("Circus_Folks",  CircusFolks);
    REG("Contort",       Contort);
    REG("Dash_Trap",     Dash_Trap);
    REG("Dog",           Dog);
    REG("Dustmaid",      Dustmaid);
    REG("Eye_Boss",      EyeBossLandPhase);
    REG("Fire_Pillar",   FirePillar);
    REG("Follower_Bro",  FollowerBro);
    REG("Four_Shooter",  Four_Shooter);
    REG("Frog",          Frog);
    REG("Gasguy",        GasGuy);
    REG("Lion",          Lion);
    REG("Mover",         Mover);
    REG("On_Off_Laser",  OnOffLaser);
    REG("Pew_Laser",     PewLaser);
    REG("Person",        Person);
    REG("Rat",           Rat);
    REG("Red_Boss",      Red_Boss);
    REG("Rotator",       Rotator);
    REG("Sage_Boss",     SageBoss);
    REG("Shieldy",       Shieldy);
    REG("Silverfish",    Silverfish);
    REG("Slasher",       Slasher);
    REG("Slime",         Slime);
    REG("Snowman",       Snowman);
    REG("Space_Face",    SpaceFace);
    REG("Spike_Roller",  SpikeRoller);
    REG("Splitboss",     SplitBoss);
    REG("Steam_Pipe",    SteamPipe);
    REG("Suburb_Killer", SuburbKiller);
    REG("Teleguy",       TeleGuy);
    REG("WallBoss",      WallBoss);
    // --- Gadgets ---
    REG("Button",        Button);
    REG("Console",       Console);
    REG("CrackedTile",   CrackedTile);
    REG("Dash_Pad",      DashPad);
    fac["Door"] = [](EntityPreset* p, Player* pl) -> std::unique_ptr<Entity> {
        if (p->TypeValue == "4") return std::make_unique<BlankPortal>(p, pl);
        return std::make_unique<Door>(p, pl);
    };
    REG("Gate",          Gate);
    REG("CardGate",      Gate);
    REG("Go_Detector",   GoDetector);
    REG("GoQuestDoorBlocker", GoHappyBlocker);
    REG("Health_Dropper",HealthEntity);
    REG("Hole",          Hole);
    REG("Key",           Key);
    REG("KeyBlock",      Key);
    REG("Pillar_Switch", PillarSwitch);
    REG("Switch_Pillar", SwitchPillar);
    REG("Treasure",      TreasureChest);
    REG("Water_Anim",    WaterAnim);
    REG("Nonsolid",      NonSolid);
    // --- Interactive (NPCs) ---
    REG("Big_Door",      Big_Door);
    REG("Black_Thing",   Black_Thing);
    REG("CubeKing",      CubeKing);
    REG("Dungeon_Statue",DungeonStatue);
    REG("Elevator",      Elevator);
    REG("Fisherman",     Fisherman);
    REG("Happy_NPC",     HappyNPC);
    REG("Health_Cicada", HealthCicada);
    REG("Huge_Fucking_Stag", HugeFuckingStag);
    REG("Mitra",         Mitra);
    REG("NPC",           Mitra);
    REG("Red_Pillar",    Red_Pillar);
    REG("Red_Walker",    RedWalker);
    REG("Redsea_NPC",    BombDude);
    REG("Sadbro",        Sadbro);
    REG("Sage",          Sage);
    REG("Shadow_Briar",  ShadowBriar);
    REG("Space_NPC",     SpaceNPC);
    REG("Suburb_Indoors",SuburbIndoors);
    REG("Suburb_Walker", SuburbWalker);
    REG("Sun_Guy",       Mitra);
    REG("Trade_NPC",     Mitra);
    REG("Forest_NPC",    Bunny);
    // --- Decorations ---
    REG("Eye_Light",     Eye_Light);
    REG("Solid_Sprite",  NonSolid);
    REG("Stop_Marker",   NonSolid);
    REG("Propelled",     NonSolid);
    REG("Jump_Trigger",  NonSolid);
    // --- Events ---
    fac["Dust"] = [](EntityPreset* p, Player*) { return std::make_unique<Dust>(p->Position); };
}

#undef REG

void EntityManager::Initialize() {
    _entities.clear();
    // Clean up old DoorPair allocations
    for (auto& [k, v] : _doorPairs) delete v;
    _doorPairs.clear();
    _linkGroups.clear();
    RegisterAllEntities();

    std::string path = Resources::ResourceManager::BaseDir + "/Content/Entities.xml";
    std::ifstream f(path);
    if (!f.is_open()) return;
    std::string xml((std::istreambuf_iterator<char>(f)),
                     std::istreambuf_iterator<char>());
    ReadEntities(xml);
}

void EntityManager::ReadEntities(const std::string& xml) {
    // Minimal line-by-line parser for the known Entities.xml format.
    // Expected lines:
    //   <map name="NAME" ...>       — start map
    //   </map>                      — end map
    //   <TypeName attr="val" .../>  — entity (self-closing)

    std::istringstream stream(xml);
    std::string line, curMap;

    // Regex for map open tag
    std::regex reMap("<map\\s+name=\"([^\"]+)\"");
    // Regex for a self-closing entity tag: captures tag name + rest of attributes
    std::regex reEntity("<(\\w[\\w_]*)\\s+([^/]*)/>"); 
    // Regex for key=value attribute pairs inside a tag
    std::regex reAttr("(\\w+)=\"([^\"]*)\"");
    std::vector<DoorMapPair> pendingDoors;

    while (std::getline(stream, line)) {
        std::smatch m;
        if (std::regex_search(line, m, reMap)) {
            curMap = m[1];
            _entities.emplace(curMap, std::vector<std::unique_ptr<EntityPreset>>{});
            continue;
        }
        if (line.find("</map>") != std::string::npos) {
            curMap.clear();
            continue;
        }
        if (curMap.empty()) continue;

        if (!std::regex_search(line, m, reEntity)) continue;

        std::string typeName = m[1];
        std::string attrStr  = m[2].str();

        // Parse all key=value attributes
        float x = 0.f, y = 0.f;
        Guid   id  = Guid::Empty;
        int    frame = 0;
        Permanence perm = Permanence::GRID_LOCAL;
        std::string typeVal;
        int  linkid = -1;
        std::string linkgroup;

        auto ai = std::sregex_iterator(attrStr.begin(), attrStr.end(), reAttr);
        auto ae = std::sregex_iterator{};
        for (; ai != ae; ++ai) {
            const std::string k = (*ai)[1], v = (*ai)[2];
            try {
                if      (k == "x")         x       = std::stof(v);
                else if (k == "y")         y       = std::stof(v);
                else if (k == "guid")      id      = Guid::Parse(v);
                else if (k == "frame")     frame   = std::stoi(v);
                else if (k == "p")         perm    = static_cast<Permanence>(std::stoi(v));
                else if (k == "type")      typeVal = v;
                else if (k == "linkid")    linkid  = std::stoi(v);
                else if (k == "linkgroup") linkgroup = v;
            } catch (...) {}
        }

        auto preset = std::make_unique<EntityPreset>(
            typeName, Vector2{x, y}, id, frame, perm, typeVal, linkid);
        EntityPreset* raw = preset.get();

        // Link group bookkeeping
        if (linkid >= 0) {
            _linkGroups[linkid].push_back(raw);
            if (!linkgroup.empty()) {
                // Parse comma-separated group IDs and merge all into one list
                std::vector<int> gids;
                std::istringstream gs(linkgroup); std::string tok;
                while (std::getline(gs, tok, ',')) {
                    try { gids.push_back(std::stoi(tok)); } catch (...) {}
                }
                // Collect all presets across the group IDs
                std::vector<EntityPreset*> merged;
                for (int gid : gids) {
                    auto git = _linkGroups.find(gid);
                    if (git != _linkGroups.end())
                        merged.insert(merged.end(), git->second.begin(), git->second.end());
                }
                // Deduplicate and write back to all group IDs
                std::sort(merged.begin(), merged.end());
                merged.erase(std::unique(merged.begin(), merged.end()), merged.end());
                for (int gid : gids) _linkGroups[gid] = merged;
            }
        }

        // Door pairing
        if (typeName == "Door") {
            DoorMapPair newDoor{raw, curMap};
            bool paired = false;
            for (int i = 0; i < (int)pendingDoors.size(); ++i) {
                if (pendingDoors[i].Door->Frame_ == frame) {
                    _doorPairs[frame] = new DoorPair(pendingDoors[i], newDoor);
                    pendingDoors.erase(pendingDoors.begin() + i);
                    paired = true;
                    break;
                }
            }
            if (!paired) pendingDoors.push_back(newDoor);
        }

        _entities[curMap].push_back(std::move(preset));
    }
}


void EntityManager::SetAlive(const Guid& id, bool isAlive) {
    auto it = State.find(id);
    if (it != State.end()) {
        it->second.Alive = isAlive;
        if (it->second.Alive && !it->second.Activated) State.erase(it);
    } else if (!isAlive) {
        State[id] = { false, false };
    }
}

void EntityManager::SetActive(const Guid& id, bool isActive) {
    auto it = State.find(id);
    if (it != State.end()) {
        it->second.Activated = isActive;
        if (it->second.Alive && !it->second.Activated) State.erase(it);
    } else if (isActive) {
        State[id] = { true, true };
    }
}

std::vector<EntityPreset*> EntityManager::GetMapEntities(const std::string& mapName) {
    auto it = _entities.find(mapName);
    if (it == _entities.end()) return {};
    std::vector<EntityPreset*> result;
    for (auto& p : it->second) result.push_back(p.get());
    return result;
}

std::vector<EntityPreset*> EntityManager::GetGridEntities(const std::string& mapName, const Point& grid) {
    auto all = GetMapEntities(mapName);
    std::vector<EntityPreset*> result;
    for (auto* p : all) {
        int gx = (int)(p->Position.X / 160);
        int gy = (int)(p->Position.Y / 160);
        if (gx == grid.X && gy == grid.Y) result.push_back(p);
    }
    return result;
}

DoorMapPair* EntityManager::GetLinkedDoor(EntityPreset* door) {
    auto it = _doorPairs.find(door->Frame_);
    if (it == _doorPairs.end()) return nullptr;
    return it->second->GetLinkedDoor(door);
}

std::vector<EntityPreset*> EntityManager::GetLinkGroup(int linkid) {
    auto it = _linkGroups.find(linkid);
    return it != _linkGroups.end() ? it->second : std::vector<EntityPreset*>{};
}

DoorMapPair* EntityManager::GetNexusGateForCurrentMap() {
    // NexusPad is a Door with type="16"
    for (auto* p : GetMapEntities(Registry::GlobalState::CURRENT_MAP_NAME))
        if (p->TypeName == "Door" && p->TypeValue == "16")
            return GetLinkedDoor(p);
    // Fall back: if NEXUS visited, return NEXUS entrance door
    if (Registry::GlobalState::events.VisitedMaps.count("NEXUS"))
        for (auto* p : GetMapEntities("NEXUS"))
            if (p->TypeName == "Door" && p->TypeValue == "17")
                return new DoorMapPair(p, "NEXUS"); // caller owns
    return nullptr;
}

} // namespace AnodyneSharp::Entities

namespace AnodyneSharp::Entities {

// EntityPreset persistence methods — access EntityManager::State
bool EntityPreset::GetAlive() const {
    auto it = EntityManager::State.find(EntityID);
    return it == EntityManager::State.end() || it->second.Alive;
}
void EntityPreset::SetAlive(bool v) {
    EntityManager::State[EntityID].Alive = v;
}
bool EntityPreset::GetActivated() const {
    auto it = EntityManager::State.find(EntityID);
    return it != EntityManager::State.end() && it->second.Activated;
}
void EntityPreset::SetActivated(bool v) {
    EntityManager::State[EntityID].Activated = v;
}

std::unique_ptr<Entity> EntityPreset::Create(Player* p) const {
    auto& facs = Factories();
    auto it = facs.find(TypeName);
    if (it == facs.end()) return nullptr;
    return it->second(const_cast<EntityPreset*>(this), p);
}

std::unordered_map<std::string, EntityPreset::FactoryFn>& EntityPreset::Factories() {
    static std::unordered_map<std::string, FactoryFn> s;
    return s;
}

} // namespace AnodyneSharp::Entities
