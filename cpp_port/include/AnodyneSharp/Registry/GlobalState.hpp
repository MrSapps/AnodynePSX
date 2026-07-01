#pragma once
#include "AnodyneSharp/Common.hpp"
#include "AnodyneSharp/Registry/Settings.hpp"
#include "AnodyneSharp/Registry/EventRegister.hpp"
#include "AnodyneSharp/Registry/InventoryManager.hpp"
#include "AnodyneSharp/Drawing/Effects/Effects.hpp"
#include "AnodyneSharp/GameTimes.hpp"

// Forward declarations to avoid circular includes
namespace AnodyneSharp {
namespace Entities { class Entity; class Player; enum class Facing; }
namespace States   { class State; class IStateSetter; }
namespace MapData  { class Map; struct IPublicMap; class Minimap; }
namespace GameEvents { struct GameEvent; }
namespace States   { class CutsceneState; }
}

namespace AnodyneSharp::Registry {

enum class GameMode { Normal, Chaos, EXTREME_CHAOS };

class GlobalState {
public:
    // --- Save/Load ---
    struct CheckPoint {
        std::string map;
        Vector2 Position = {0,0};
        CheckPoint() = default;
        CheckPoint(const std::string& m, Vector2 pos) : map(m), Position(pos) {}
        void Warp(Vector2 offset);
    };

    static std::string CurrentSaveGame;
    static std::string serialized_quicksave;
    static std::optional<CheckPoint> quicksave_checkpoint;

    static void SaveGame(const std::string& id = "");
    static void LoadSave(const std::string& savId);
    static std::string SerializeToString();
    static void DeserializeFromString(const std::string& data);
    static void ResetValues();

    // --- Dialogue ---
    static std::string Dialogue_value;
    static void  SetDialogue(const std::string& val);
    static const std::string& GetDialogue();
    static bool  DialogueTop;
    static bool  LastDialogueFinished;
    static bool  SetDialogueMode;

    // --- Map / Grid ---
    static std::string CURRENT_MAP_NAME;
    static std::string NEXT_MAP_NAME;
    static int   CURRENT_GRID_X;
    static int   CURRENT_GRID_Y;
    static int   MAP_GRID_WIDTH;
    static int   MAP_GRID_HEIGHT;
    static Point CurrentMapGrid() { return {CURRENT_GRID_X, CURRENT_GRID_Y}; }
    static Point TopLeftTile()    { return {CURRENT_GRID_X*10, CURRENT_GRID_Y*10}; }
    static Rectangle ScreenHitbox(){ return {CURRENT_GRID_X*160, CURRENT_GRID_Y*160, 160, 160}; }
    static bool  ScreenTransition;

    static int   ENEMIES_KILLED;
    static int   PUZZLES_SOLVED;

    static MapData::IPublicMap* Map;

    // --- State callbacks ---
    static States::IStateSetter* GameState;
    static std::function<void(Entities::Entity*)>        SpawnEntity;
    static std::function<void(GameEvents::GameEvent*)>   FireEvent;
    static std::function<void(States::State*)>           SetSubstate;
    static std::function<void()>  DoQuickSave;
    static std::function<void()>  DoQuickLoad;

    // --- Registry ---
    static Settings       settings;
    static EventRegister  events;
    static InventoryManager inventory;
    static bool           ResolutionDirty;
    static Dialogue::Language CurrentLanguage() { return settings.language; }

    static int   PillarSwitchOn;
    static int   ENEMIES_KILLED_GLOBAL;

    // --- Warp ---
    static bool   WARP;
    static Vector2 PLAYER_WARP_TARGET;
    static std::optional<Entities::Facing> NewMapFacing;
    static bool   RefreshKeyCount;
    static bool   RefreshMaxHealth;
    static bool   RefreshLabels;

    // --- Health ---
    static int  CUR_HEALTH;
    static int  MAX_HEALTH_get();
    static void MAX_HEALTH_set(int v);
    static std::string DamageDealer;
    static int  DeathCount;

    // --- Flags ---
    static bool AlwaysCellGraphics;
    static GameMode GameMode_;
    static bool ForceTextureReload;
    static bool ShowFPS;
    static bool ClosingGame;
    static bool ToTitle;
    static bool BoiEaster;
    static bool disable_menu;
    static bool FUCK_IT_MODE_ON;
    static bool draw_hitboxes;
    static bool InDeathRoom;
    static bool IsCell()  { return AlwaysCellGraphics || CURRENT_MAP_NAME=="CELL"; }
    static bool IsKnife() { return CURRENT_MAP_NAME == "SUBURB"; }
    static bool IsDungeon();
    static bool CanChangeBroom();

    // --- Minimap ---
    static MapData::Minimap* CurrentMinimap;

    // --- RNG ---
    static Random RNG;

    // --- Playtime ---
    static std::chrono::system_clock::time_point START_TIME;
    static std::chrono::milliseconds _totalPreviously;
    static std::chrono::milliseconds PlayTime();

    // --- Check/Return point ---
    static std::optional<CheckPoint> checkpoint;
    static std::optional<CheckPoint> ReturnTarget;

    // --- Full screen effects ---
    static Drawing::Effects::Darkness          darkness;
    static Drawing::Effects::FadeEffect        gameScreenFade;
    static Drawing::Effects::TitleScreenOverlay TitleScreenFinish;
    static Drawing::Effects::Static            staticEffect;
    static Drawing::Effects::ScreenShake       screenShake;
    static Drawing::Effects::FadeEffect        black_overlay;
    static Drawing::Effects::FlashEffect       flash;
    static Drawing::Effects::Pixelate          pixelation;
    static Drawing::Effects::FG_Blend          fgBlend;
    static Drawing::Effects::BlendEffect       extraBlend;
    static Drawing::Effects::Wave              wave;
    static Drawing::Effects::Glitch            glitch;
    static Drawing::Effects::GrayScale         grayScale;

    static std::vector<IFullScreenEffect*> gameEffects;
    static std::vector<IFullScreenEffect*> fullScreenEffects;
    static std::vector<IFullScreenEffect*> AllEffects();

    static std::vector<Entities::Entity*> UIEntities;

    // --- Player light ---
    static Entities::Entity* PlayerLight;

    // --- Cutscene ---
    // using Coroutine = ... but CutsceneState not fully defined here; use void*
    static void* StartCutscene;

private:
    static int _curHealth;
    static int _maxHealth;
    static std::string _dialogue;
};

} // namespace AnodyneSharp::Registry

using AnodyneSharp::Registry::GlobalState;
using AnodyneSharp::Registry::GameMode;
