#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/Drawing/Effects/Effects.hpp"
#include "AnodyneSharp/Entities/Base/Entity.hpp"
#include "AnodyneSharp/MapData/Map.hpp"
#include "AnodyneSharp/Dialogue/DialogueManager.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace AnodyneSharp::Registry {

// Static member definitions
std::string GlobalState::CurrentSaveGame    = "1";
std::string GlobalState::serialized_quicksave;
std::optional<GlobalState::CheckPoint> GlobalState::quicksave_checkpoint;
std::string GlobalState::Dialogue_value;
bool GlobalState::DialogueTop            = false;
bool GlobalState::LastDialogueFinished   = true;
bool GlobalState::SetDialogueMode        = false;

std::string GlobalState::CURRENT_MAP_NAME;
std::string GlobalState::NEXT_MAP_NAME    = "BLANK";
int  GlobalState::CURRENT_GRID_X          = 0;
int  GlobalState::CURRENT_GRID_Y          = 0;
int  GlobalState::MAP_GRID_WIDTH          = 0;
int  GlobalState::MAP_GRID_HEIGHT         = 0;
bool GlobalState::ScreenTransition        = false;
int  GlobalState::ENEMIES_KILLED          = 0;
int  GlobalState::PUZZLES_SOLVED          = 0;
int  GlobalState::ENEMIES_KILLED_GLOBAL   = 0;

MapData::IPublicMap*  GlobalState::Map      = nullptr;
States::IStateSetter* GlobalState::GameState = nullptr;
std::function<void(Entities::Entity*)>       GlobalState::SpawnEntity;
std::function<void(GameEvents::GameEvent*)>  GlobalState::FireEvent;
std::function<void(States::State*)>          GlobalState::SetSubstate;
std::function<void()>  GlobalState::DoQuickSave;
std::function<void()>  GlobalState::DoQuickLoad;

Settings        GlobalState::settings;
EventRegister   GlobalState::events;
InventoryManager GlobalState::inventory;
bool            GlobalState::ResolutionDirty = false;
int             GlobalState::PillarSwitchOn  = 0;

bool   GlobalState::WARP                = false;
Vector2 GlobalState::PLAYER_WARP_TARGET = {23.f, 110.f};
std::optional<Entities::Facing> GlobalState::NewMapFacing;

bool GlobalState::RefreshKeyCount  = false;
bool GlobalState::RefreshMaxHealth = false;
bool GlobalState::RefreshLabels    = false;

int  GlobalState::CUR_HEALTH = 6;
std::string GlobalState::DamageDealer;
int  GlobalState::DeathCount = 0;
int  GlobalState::_curHealth = 6;
int  GlobalState::_maxHealth = 6;
std::string GlobalState::_dialogue;

bool GlobalState::AlwaysCellGraphics = false;
GameMode GlobalState::GameMode_ = GameMode::Normal;
bool GlobalState::ForceTextureReload = false;
bool GlobalState::ShowFPS        = false;
bool GlobalState::ClosingGame    = false;
bool GlobalState::ToTitle        = false;
bool GlobalState::BoiEaster      = false;
bool GlobalState::disable_menu   = false;
bool GlobalState::FUCK_IT_MODE_ON = false;
bool GlobalState::draw_hitboxes  = false;
bool GlobalState::InDeathRoom    = false;

Random GlobalState::RNG;

std::chrono::system_clock::time_point GlobalState::START_TIME;
std::chrono::milliseconds GlobalState::_totalPreviously{0};

std::optional<GlobalState::CheckPoint> GlobalState::checkpoint;
std::optional<GlobalState::CheckPoint> GlobalState::ReturnTarget;

MapData::Minimap* GlobalState::CurrentMinimap = nullptr;
Entities::Entity* GlobalState::PlayerLight = nullptr;
void* GlobalState::StartCutscene = nullptr;

std::vector<Entities::Entity*> GlobalState::UIEntities;

// Effect instances
Drawing::Effects::Darkness           GlobalState::darkness;
Drawing::Effects::FadeEffect         GlobalState::gameScreenFade;
Drawing::Effects::TitleScreenOverlay GlobalState::TitleScreenFinish;
Drawing::Effects::Static             GlobalState::staticEffect;
Drawing::Effects::ScreenShake        GlobalState::screenShake;
Drawing::Effects::FadeEffect         GlobalState::black_overlay;
Drawing::Effects::FlashEffect        GlobalState::flash;
Drawing::Effects::Pixelate           GlobalState::pixelation;
Drawing::Effects::FG_Blend           GlobalState::fgBlend;
Drawing::Effects::BlendEffect        GlobalState::extraBlend;
Drawing::Effects::Wave               GlobalState::wave;
Drawing::Effects::Glitch             GlobalState::glitch;
Drawing::Effects::GrayScale          GlobalState::grayScale;

std::vector<IFullScreenEffect*> GlobalState::gameEffects;
std::vector<IFullScreenEffect*> GlobalState::fullScreenEffects;

// Methods
void GlobalState::SetDialogue(const std::string& val) {
    _dialogue        = val;
    SetDialogueMode  = !val.empty();
    LastDialogueFinished = val.empty();
    if (val.empty()) DialogueTop = false;
}

const std::string& GlobalState::GetDialogue() { return _dialogue; }

void GlobalState::ResetValues() {
    START_TIME = std::chrono::system_clock::now();
    CURRENT_MAP_NAME = "";
    NEXT_MAP_NAME    = "BLANK";
    PLAYER_WARP_TARGET = {23.f, 110.f};
    NewMapFacing = Entities::Facing::RIGHT;
    _maxHealth = 6;
    _curHealth = 6;
    DeathCount = 0;
    RefreshKeyCount  = true;
    RefreshMaxHealth = true;
    AlwaysCellGraphics = false;
    GameMode_ = GameMode::Normal;
    InDeathRoom = false;
    disable_menu = false;

    events    = EventRegister{};
    inventory = InventoryManager{};

    // Initialize effect lists
    gameEffects   = { &fgBlend, &staticEffect, &darkness, &gameScreenFade };
    fullScreenEffects = { &black_overlay, &glitch, &grayScale, &TitleScreenFinish,
                          &pixelation, &extraBlend, &wave, &flash, &screenShake };
}

std::vector<IFullScreenEffect*> GlobalState::AllEffects() {
    std::vector<IFullScreenEffect*> all = gameEffects;
    all.insert(all.end(), fullScreenEffects.begin(), fullScreenEffects.end());
    return all;
}

int GlobalState::MAX_HEALTH_get() { return _maxHealth; }
void GlobalState::MAX_HEALTH_set(int v) {
    _maxHealth = v <= 16 ? v : 16;
    RefreshMaxHealth = true;
}

bool GlobalState::IsDungeon() {
    return ReturnTarget.has_value() && ReturnTarget->map != "NEXUS";
}

bool GlobalState::CanChangeBroom() {
    return !InDeathRoom && !IsCell() && !IsKnife();
}

std::chrono::milliseconds GlobalState::PlayTime() {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - START_TIME);
    return _totalPreviously + elapsed;
}

void GlobalState::SaveGame(const std::string& id) {
    std::string saveId = id.empty() ? CurrentSaveGame : id;
    std::string dir    = GameConstants::SavePath + "Saves/";
    // Best-effort directory creation on Windows
    std::string path = dir + "Save_" + saveId + ".dat";
    std::ofstream f(path);
    if (!f.is_open()) {
        // Try creating directory first
        std::string mkdirCmd = "mkdir \"" + dir + "\" 2>nul";
        std::system(mkdirCmd.c_str());
        f.open(path);
        if (!f.is_open()) return;
    }

    // Health
    f << "cur_health=" << CUR_HEALTH << "\n";
    f << "max_health=" << MAX_HEALTH_get() << "\n";
    f << "deaths="     << DeathCount << "\n";
    f << "playtime="   << PlayTime().count() << "\n";

    // Checkpoint
    if (checkpoint) {
        f << "checkpoint_map="  << checkpoint->map         << "\n";
        f << "checkpoint_x="    << checkpoint->Position.X  << "\n";
        f << "checkpoint_y="    << checkpoint->Position.Y  << "\n";
    }

    // Inventory
    f << "inv_broom="      << (inventory.HasBroom       ? "1" : "0") << "\n";
    f << "inv_lengthen="   << (inventory.HasLengthen    ? "1" : "0") << "\n";
    f << "inv_widen="      << (inventory.HasWiden       ? "1" : "0") << "\n";
    f << "inv_transformer="<< (inventory.HasTransformer ? "1" : "0") << "\n";
    f << "inv_canjump="    << (inventory.CanJump        ? "1" : "0") << "\n";
    f << "inv_equipped="   << (int)inventory.EquippedBroom()         << "\n";
    f << "inv_tradestate=" << (int)inventory.tradeState              << "\n";
    // Map keys
    for (auto& [m, k] : inventory._mapKeys) f << "mapkey_" << m << "=" << k << "\n";
    // Cards
    for (int i = 0; i < (int)inventory.CardStatus.size(); i++)
        if (inventory.CardStatus[i]) f << "card_" << i << "=1\n";
    // Big keys
    for (int i = 0; i < (int)inventory.BigKeyStatus.size(); i++)
        if (inventory.BigKeyStatus[i]) f << "bigkey_" << i << "=1\n";

    // Events
    for (auto& m  : events.VisitedMaps)          f << "visited_map=" << m  << "\n";
    for (auto& m  : events.BossDefeated)         f << "boss_dead="   << m  << "\n";
    for (auto& [k,v] : events.eventKeys)         f << "event_" << k << "=" << v << "\n";

    // Dialogue scene progress (only non-default states)
    for (auto& [npc, npcData] : Dialogue::DialogueManager::SceneTree)
        for (auto& [area, areaData] : npcData.areas)
            for (auto& [scn, scnData] : areaData.scenes) {
                auto& st = scnData.state;
                if (st.line != 0 || st.dirty || st.finished)
                    f << "dlg_" << npc << "/" << area << "/" << scn
                      << "=" << st.line << "/"
                      << (st.dirty    ? "1" : "0") << "/"
                      << (st.finished ? "1" : "0") << "\n";
            }
}

void GlobalState::LoadSave(const std::string& savId) {
    std::string path = GameConstants::SavePath + "Saves/Save_" + savId + ".dat";
    std::ifstream f(path);
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        auto ibool  = [&](){ return val == "1"; };
        auto iint   = [&](){ try{return std::stoi(val);}catch(...){return 0;} };
        auto ifloat = [&](){ try{return std::stof(val);}catch(...){return 0.f;} };
        if      (key == "cur_health")    CUR_HEALTH  = iint();
        else if (key == "max_health")    MAX_HEALTH_set(iint());
        else if (key == "deaths")        DeathCount  = iint();
        else if (key == "playtime")      _totalPreviously = std::chrono::milliseconds(iint());
        else if (key == "checkpoint_map")  { if (!checkpoint) checkpoint = CheckPoint{}; checkpoint->map = val; }
        else if (key == "checkpoint_x")    { if (!checkpoint) checkpoint = CheckPoint{}; checkpoint->Position.X = ifloat(); }
        else if (key == "checkpoint_y")    { if (!checkpoint) checkpoint = CheckPoint{}; checkpoint->Position.Y = ifloat(); }
        else if (key == "inv_broom")       inventory.HasBroom       = ibool();
        else if (key == "inv_lengthen")    inventory.HasLengthen    = ibool();
        else if (key == "inv_widen")       inventory.HasWiden       = ibool();
        else if (key == "inv_transformer") inventory.HasTransformer = ibool();
        else if (key == "inv_canjump")     inventory.CanJump        = ibool();
        else if (key == "inv_equipped")    inventory.SetEquippedBroom((Entities::BroomType)iint());
        else if (key == "inv_tradestate")  inventory.tradeState     = (InventoryManager::TradeState)iint();
        else if (key.substr(0,7) == "mapkey_") inventory._mapKeys[key.substr(7)] = iint();
        else if (key.substr(0,5) == "card_")   { int i=std::stoi(key.substr(5)); if(i<49) inventory.CardStatus[i]=true; }
        else if (key.substr(0,7) == "bigkey_") { int i=std::stoi(key.substr(7)); if(i<3)  inventory.BigKeyStatus[i]=true; }
        else if (key == "visited_map")     events.VisitedMaps.insert(val);
        else if (key == "boss_dead")       events.BossDefeated.insert(val);
        else if (key.substr(0,6) == "event_") events.eventKeys[key.substr(6)] = iint();
        else if (key.substr(0,4) == "dlg_") {
            // Format: dlg_npc/area/scene = line/dirty/finished
            std::string k2 = key.substr(4);
            auto s1 = k2.find('/');
            auto s2 = (s1 != std::string::npos) ? k2.find('/', s1+1) : std::string::npos;
            if (s1 != std::string::npos && s2 != std::string::npos) {
                std::string npc  = k2.substr(0, s1);
                std::string area = k2.substr(s1+1, s2-s1-1);
                std::string scn  = k2.substr(s2+1);
                auto v1 = val.find('/');
                auto v2 = (v1 != std::string::npos) ? val.find('/', v1+1) : std::string::npos;
                if (v1 != std::string::npos && v2 != std::string::npos) {
                    int  dline     = 0;
                    bool ddirty    = val[v1+1] == '1';
                    bool dfinished = val[v2+1] == '1';
                    try { dline = std::stoi(val.substr(0, v1)); } catch (...) {}
                    auto ni = Dialogue::DialogueManager::SceneTree.find(npc);
                    if (ni != Dialogue::DialogueManager::SceneTree.end()) {
                        auto* da = ni->second.GetArea(area);
                        if (da) {
                            auto* ds = da->GetScene(scn);
                            if (ds) ds->state = { dline, ddirty, dfinished };
                        }
                    }
                }
            }
        }
    }
    if (checkpoint) {
        PLAYER_WARP_TARGET = checkpoint->Position;
        NEXT_MAP_NAME      = checkpoint->map;
        WARP               = true;
    }
}

void GlobalState::CheckPoint::Warp(Vector2 offset) {
    GlobalState::NEXT_MAP_NAME    = map;
    GlobalState::PLAYER_WARP_TARGET = {Position.X + offset.X, Position.Y + offset.Y};
    GlobalState::WARP = true;
}

// Shared serialization helper — writes the full save body to any ostream
static void WriteStateToStream(std::ostream& f) {
    using namespace AnodyneSharp::Registry;
    using GS = GlobalState;
    // Health
    f << "cur_health=" << GS::CUR_HEALTH << "\n";
    f << "max_health=" << GS::MAX_HEALTH_get() << "\n";
    f << "deaths="     << GS::DeathCount << "\n";
    f << "playtime="   << GS::PlayTime().count() << "\n";
    // Checkpoint
    if (GS::checkpoint) {
        f << "checkpoint_map="  << GS::checkpoint->map        << "\n";
        f << "checkpoint_x="    << GS::checkpoint->Position.X << "\n";
        f << "checkpoint_y="    << GS::checkpoint->Position.Y << "\n";
    }
    // Inventory
    auto& inv = GS::inventory;
    f << "inv_broom="       << (inv.HasBroom       ? "1" : "0") << "\n";
    f << "inv_lengthen="    << (inv.HasLengthen    ? "1" : "0") << "\n";
    f << "inv_widen="       << (inv.HasWiden       ? "1" : "0") << "\n";
    f << "inv_transformer=" << (inv.HasTransformer ? "1" : "0") << "\n";
    f << "inv_canjump="     << (inv.CanJump        ? "1" : "0") << "\n";
    f << "inv_equipped="    << (int)inv.EquippedBroom()         << "\n";
    f << "inv_tradestate="  << (int)inv.tradeState              << "\n";
    for (auto& [m, k] : inv._mapKeys)       f << "mapkey_" << m << "=" << k << "\n";
    for (int i = 0; i < (int)inv.CardStatus.size(); i++)
        if (inv.CardStatus[i]) f << "card_" << i << "=1\n";
    for (int i = 0; i < (int)inv.BigKeyStatus.size(); i++)
        if (inv.BigKeyStatus[i]) f << "bigkey_" << i << "=1\n";
    // Events
    for (auto& m : GS::events.VisitedMaps)        f << "visited_map=" << m << "\n";
    for (auto& m : GS::events.BossDefeated)       f << "boss_dead="   << m << "\n";
    for (auto& [k, v] : GS::events.eventKeys)     f << "event_" << k << "=" << v << "\n";
    // Dialogue
    for (auto& [npc, npcData] : AnodyneSharp::Dialogue::DialogueManager::SceneTree)
        for (auto& [area, areaData] : npcData.areas)
            for (auto& [scn, scnData] : areaData.scenes) {
                auto& st = scnData.state;
                if (st.line != 0 || st.dirty || st.finished)
                    f << "dlg_" << npc << "/" << area << "/" << scn
                      << "=" << st.line << "/" << (st.dirty?"1":"0") << "/" << (st.finished?"1":"0") << "\n";
            }
}

std::string GlobalState::SerializeToString() {
    std::ostringstream ss;
    WriteStateToStream(ss);
    return ss.str();
}

void GlobalState::DeserializeFromString(const std::string& data) {
    std::istringstream f(data);
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        if (!val.empty() && val.back() == '\r') val.pop_back();
        auto ibool  = [&](){ return val == "1"; };
        auto iint   = [&](){ try{return std::stoi(val);}catch(...){return 0;} };
        auto ifloat = [&](){ try{return std::stof(val);}catch(...){return 0.f;} };
        if      (key == "cur_health")      CUR_HEALTH = iint();
        else if (key == "max_health")      MAX_HEALTH_set(iint());
        else if (key == "deaths")          DeathCount = iint();
        else if (key == "playtime")        _totalPreviously = std::chrono::milliseconds((long long)iint());
        else if (key == "checkpoint_map")  { if (!checkpoint) checkpoint = CheckPoint{}; checkpoint->map = val; }
        else if (key == "checkpoint_x")    { if (!checkpoint) checkpoint = CheckPoint{}; checkpoint->Position.X = ifloat(); }
        else if (key == "checkpoint_y")    { if (!checkpoint) checkpoint = CheckPoint{}; checkpoint->Position.Y = ifloat(); }
        else if (key == "inv_broom")       inventory.HasBroom       = ibool();
        else if (key == "inv_lengthen")    inventory.HasLengthen    = ibool();
        else if (key == "inv_widen")       inventory.HasWiden       = ibool();
        else if (key == "inv_transformer") inventory.HasTransformer = ibool();
        else if (key == "inv_canjump")     inventory.CanJump        = ibool();
        else if (key == "inv_equipped")    inventory.SetEquippedBroom((Entities::BroomType)iint());
        else if (key == "inv_tradestate")  inventory.tradeState     = (InventoryManager::TradeState)iint();
        else if (key.size() > 7 && key.substr(0,7) == "mapkey_") inventory._mapKeys[key.substr(7)] = iint();
        else if (key.size() > 5 && key.substr(0,5) == "card_")   { int i=std::stoi(key.substr(5)); if(i<49) inventory.CardStatus[i]=true; }
        else if (key.size() > 7 && key.substr(0,7) == "bigkey_") { int i=std::stoi(key.substr(7)); if(i<3)  inventory.BigKeyStatus[i]=true; }
        else if (key == "visited_map")     events.VisitedMaps.insert(val);
        else if (key == "boss_dead")       events.BossDefeated.insert(val);
        else if (key.size() > 6 && key.substr(0,6) == "event_") events.eventKeys[key.substr(6)] = iint();
        else if (key.size() > 4 && key.substr(0,4) == "dlg_") {
            std::string k2 = key.substr(4);
            auto s1 = k2.find('/'); auto s2 = (s1!=std::string::npos) ? k2.find('/',s1+1) : std::string::npos;
            if (s1!=std::string::npos && s2!=std::string::npos) {
                std::string npc=k2.substr(0,s1), area=k2.substr(s1+1,s2-s1-1), scn=k2.substr(s2+1);
                auto v1=val.find('/'); auto v2=(v1!=std::string::npos)?val.find('/',v1+1):std::string::npos;
                if (v1!=std::string::npos && v2!=std::string::npos) {
                    int dl=0; bool dd=val[v1+1]=='1', df=val[v2+1]=='1';
                    try{dl=std::stoi(val.substr(0,v1));}catch(...){}
                    auto ni=Dialogue::DialogueManager::SceneTree.find(npc);
                    if(ni!=Dialogue::DialogueManager::SceneTree.end()){
                        auto* da=ni->second.GetArea(area);
                        if(da){auto* ds=da->GetScene(scn);if(ds)ds->state={dl,dd,df};}
                    }
                }
            }
        }
    }
}

} // namespace AnodyneSharp::Registry
