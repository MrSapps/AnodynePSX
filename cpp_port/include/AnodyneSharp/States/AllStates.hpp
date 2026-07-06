#pragma once
#include "AnodyneSharp/States/Base/State.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Registry/InventoryManager.hpp"
#include "AnodyneSharp/Entities/CollisionGroups.hpp"
#include "AnodyneSharp/MapData/Map.hpp"
#include "AnodyneSharp/Coroutine.hpp"
#include "AnodyneSharp/UI/UITypes.hpp"
#include "AnodyneSharp/UI/PauseMenu/PauseMenu.hpp"
#include "AnodyneSharp/Entities/Decorations/Decorations.hpp"
#include "AnodyneSharp/Utilities/MapUtilities.hpp"
#include "AnodyneSharp/Drawing/ScrollingTex.hpp"
#include <vector>
#include <memory>

namespace AnodyneSharp::States {

// Forward declarations for all state classes
class TitleState;
class MainMenuState;
class PlayState;
class CreditsState;
class DeathState;
class DialogueState;
class IntroState;
class CutsceneState;
class PauseState;

// ---- PlayState - the main game state ----
class PlayState : public State {
public:
    enum class PSState { S_LOAD, S_NORMAL, S_TRANSITION, S_MAP_EXIT, S_MAP_ENTER };

    PlayState();

    void Create()     override;
    void Initialize() override;
    void Update()     override;
    void Draw()       override;
    void DrawUI()     override;

    static void Reset();
    static bool HasBeenReset();

private:
    void SetBackground();

    std::unique_ptr<Entities::Player>              _player;
    std::unique_ptr<MapData::Map>                  _map;
    std::unique_ptr<Entities::CollisionGroups>     _collision;
    std::vector<std::unique_ptr<Entities::Entity>> _entities;
    std::vector<State*>                            _childStates;

    Rectangle      _gridBorders{};
    PSState        _psState = PSState::S_LOAD;
    UI::HealthBar  _healthBar;
    AnodyneSharp::Drawing::ScrollingTex   _background;

    void SpawnEntities();
    void UpdateEntitiesFn();
    void DoMapTransition();
    void DoCollisions(bool ignorePlayer);
    void UpdateScreenBorders();
    void Warp();
    bool CheckInteraction();
    void QuickSave();
    void QuickLoad();
    void MapSpecificUpdate();

    bool  _doQuickLoad    = false;
    float _quickSaveTimer = 5.f;  // prevents spam after loading
};

// ---- DialogueState ----
class DialogueState : public State {
public:
    DialogueState(const std::string& text, bool useMenuBox = false, bool isIntro = false);

    void Update()  override;
    void DrawUI()  override;

private:
    enum class DSState { WRITING, WAITING };
    DSState    _dsState    = DSState::WRITING;
    UI::TextBox _tb;          // ctor args forwarded in initializer list
    int        _normalSpeed = 30;
    int        _speedScale  = 2;
};

// ---- DeathState ----
class DeathState : public State {
public:
    DeathState(Entities::Player* player);

    void Update()  override;
    void DrawUI()  override;

private:
    enum class DState { FADE_IN, SELECT, FADE_OUT };
    DState _dState     = DState::FADE_IN;
    Entities::Player* _player = nullptr;
    std::unique_ptr<Entities::PlayerDieDummy> _dummy;
    std::unique_ptr<Entities::DeathFadeIn>    _deathFade;
    std::unique_ptr<UILabel> _continueLabel;
    std::unique_ptr<UILabel> _yesLabel;
    std::unique_ptr<UILabel> _noLabel;
    bool _yesSelected = true;
    bool _gotControl  = false;
};

// ---- CreditsState ----
class CreditsState : public State {
public:
    CreditsState();
    void Update()  override;
    void DrawUI()  override;
private:
    static constexpr int MAX_LABELS = 28;
    static constexpr float SCROLL_SPEED = 15.f;
    std::vector<std::unique_ptr<UILabel>> _labels;
    std::unique_ptr<UILabel> _endLabel;
    bool   _stopScroll = false;
    bool   _waitingAccept = false;
};

// ---- IntroState ----
class IntroState : public State {
public:
    IntroState();
    void Update()  override;
    void DrawUI()  override;
private:
    enum class IState { WaitStart, Writing, WaitEnd };
    IState _istate = IState::WaitStart;
    float  _timer  = 1.5f;
    std::unique_ptr<DialogueState> _dialogueSub;
};

// ---- TitleState ----
class TitleState : public State {
public:
    TitleState();
    void Create()  override;
    void Update()  override;
    void Draw()    override;
    void DrawUI()  override;

private:
    Drawing::ScrollingTex _background;

    std::unique_ptr<UIEntity> mTitleTex;
    std::unique_ptr<UIEntity> mTitleOverlay;    

    std::unique_ptr<UIEntity> mSubtitle;
    std::unique_ptr<UIEntity> mSubtitleOverlay;

    std::unique_ptr<UIEntity> mNexusImage;

    std::unique_ptr<UIEntity> mDoorGlow;
    std::unique_ptr<UIEntity> mDoorSpin1;
    std::unique_ptr<UIEntity> mDoorSpin2;

    std::unique_ptr<UIEntity> mPressEnterTex;

    bool  _pressEnterVisible = false;
    float _blinkTimer        = 1.f;
    bool  _pixelating        = false;
};

class Substate;  // forward declaration for MainMenuState

// ---- MainMenuState ----
class MainMenuState : public State {
public:
    MainMenuState();
    void Update()  override;
    void DrawUI()  override;
private:
    static int _state;
    std::vector<std::unique_ptr<UILabel>> _labels;
    std::unique_ptr<Substate> _substate;
    int  _lastState = 0;
    bool _inSubstate = false;
    MenuSelector _selector;

    void StateChanged();
    std::unique_ptr<Substate> CreateSubstate(int idx);
};

// ---- Substate base class (defined before PauseState so PauseState can use it) ----
class Substate : public State {
public:
    Substate();
    virtual void HandleInput();   // default: Cancel/Left → ExitSubState
    virtual void GetControl();    // show selector
    void Update()  override;
    void DrawUI()  override;
protected:
    virtual void OnExit() {}
    void ExitSubState();
    MenuSelector _selector;
};

// DialogueSubstate — extends Substate with an inner DialogueState pop-up
class DialogueSubstate : public Substate {
public:
    void Update()  override;
    void DrawUI()  override;
    void HandleInput() override;
protected:
    bool InDialogueMode() const { return _dialogueSub != nullptr; }
    void SetDialogue(const std::string& text);
private:
    std::unique_ptr<DialogueState> _dialogueSub;
};

// ---- SaveSubstate ----
class SaveSubstate : public Substate {
public:
    SaveSubstate();
    void Update()     override;
    void HandleInput() override;
    void DrawUI()     override;
    void GetControl() override;
private:
    enum class SaveAction { QuickSave=0, QuickLoad, Save, SaveTitle, SaveQuit, Quit, COUNT };
    SaveAction _curAction  = SaveAction::QuickSave;
    SaveAction _lastAction = SaveAction::QuickSave;
    std::vector<std::unique_ptr<UILabel>> _labels;  // 6 action + 1 deaths
    void SetSelectorPos();
};

// ---- MapSubstate ----
class MapSubstate : public Substate {
public:
    MapSubstate();
    void Update()     override;
    void HandleInput() override;
    void DrawUI()     override;
    void GetControl() override;
private:
    enum class MapNav { Return, No, Yes };
    MapNav _nav = MapNav::Return;
    MapNav _lastNav = MapNav::Return;
    float  _blinkTimer = 0.f;
    static constexpr float _playerBlink = 0.4f;
    Drawing::Spritesheet::Spritesheet _mapSheet;
    std::unique_ptr<UILabel> _returnLabel;
    std::unique_ptr<UILabel> _yesLabel;
    std::unique_ptr<UILabel> _noLabel;
    std::unique_ptr<UILabel> _noMapLabel;
    void SetSelectorPos();
};

// ---- EquipSubstate ----
class EquipSubstate : public DialogueSubstate {
public:
    EquipSubstate();
    void Update()     override;
    void HandleInput() override;
    void DrawUI()     override;
    void GetControl() override;
private:
    enum class EquipItem { Broom=0, Extend, Widen, Transformer, Shoes, TradeItem, Key1, Key2, Key3 };
    EquipItem _state = EquipItem::Broom;
    EquipItem _last  = EquipItem::Broom;
    std::vector<EquipItem> _bottomRow;
    int _bottomIdx = 0;
    std::vector<std::unique_ptr<UILabel>> _broomLabels;  // Broom, Extend, Widen, Transformer
    void SetSelectorPos();
    void UseItem();
    void EquipBroom(Entities::BroomType bt);
};

// ---- CardSubstate ----
class CardSubstate : public DialogueSubstate {
public:
    CardSubstate();
    void Update()     override;
    void HandleInput() override;
    void DrawUI()     override;
    void GetControl() override;
private:
    int _page     = 0;
    int _selected = 0;  // negative = page-selector row
    std::unique_ptr<UILabel> _cardsLabel;
    std::unique_ptr<UILabel> _pageLabel;
    void SetCardPage();
};

// ---- ConfigSubstate ----
class ConfigSubstate : public Substate {
public:
    ConfigSubstate();
    void Update()     override;
    void HandleInput() override;
    void DrawUI()     override;
    void GetControl() override;
protected:
    void OnExit() override;
private:
    enum class ConfigItem { MusicVol=0, SFXVol, Language, COUNT };
    ConfigItem _cur  = ConfigItem::MusicVol;
    ConfigItem _last = ConfigItem::MusicVol;
    std::vector<std::unique_ptr<UILabel>> _labels;
    void SetSelectorPos();
};

// ---- Stubs for less-critical substates ----
class CheatzSubstate   : public Substate {};
class SecretSubstate   : public Substate {};
class ControlsSubstate : public Substate {};
class GameplayMenu     : public Substate {};
class GraphicsMenu     : public Substate {};
// ---- FileSubstate — save slot selector ----
class FileSubstate : public Substate {
public:
    explicit FileSubstate(int saveId);
    void Update()     override;
    void HandleInput() override;
    void DrawUI()     override;
    void GetControl() override;
private:
    enum class FSState { Game, NewGame, YesConfirm, NoConfirm };
    int     _saveId;
    bool    _saveExists  = false;
    bool    _loadedSave  = false;
    bool    _newSave     = false;
    bool    _confirming  = false;
    FSState _fstate = FSState::Game;
    FSState _lastFState = FSState::Game;
    std::vector<std::unique_ptr<UILabel>> _labels;  // Game, New Game, [Yes, No]
    void SetSelectorPos();
    void ExecuteLoad();
    void ExecuteNew();
    static std::string SavePath(int id);
};

// ---- PauseState (substate of PlayState) ----
class PauseState : public State {
public:
    PauseState();
    ~PauseState();
    void Update()  override;
    void DrawUI()  override;
    static void Reset();

private:
    static int _menuState;
    std::vector<std::unique_ptr<UILabel>> _menuLabels;
    std::unique_ptr<UILabel> _playtimeLabel;
    std::unique_ptr<Substate> _substate;
    int  _lastState    = 0;
    bool _inSubstate   = false;
    int  _cheatCounter = 0;

    static constexpr int CHEAT_MAX = 20;
    static constexpr int MENU_COUNT = 5;  // Map, Items, Cards, Save, Config

    void StateChanged();
    std::unique_ptr<Substate> CreateSubstate(int idx);
};

// ---- CutsceneState ----
class CutsceneState : public State {
public:
    struct CutsceneEvent { virtual ~CutsceneEvent() = default; };
    struct DialogueEvent : CutsceneEvent { std::string Diag; DialogueEvent(const std::string& d) : Diag(d) {} };
    struct WarpEvent     : CutsceneEvent { std::string MapName; Point Grid; };
    struct ReturnWarp    : CutsceneEvent {};
    struct EntityEvent   : CutsceneEvent { std::vector<Entities::Entity*> NewEntities; };

    CutsceneState(Coroutine stateCoroutine);
    void Update()  override;
    void Draw()    override;
    void DrawUI()  override;

    void Return();
    void Warp(const std::string& map, const Point& grid);

private:
    MapData::Map* _map = nullptr;
    std::vector<Entities::Entity*> _entities;
    float _oldDarkness = 0.f;
};

} // namespace AnodyneSharp::States

using AnodyneSharp::States::PlayState;
using AnodyneSharp::States::TitleState;
using AnodyneSharp::States::MainMenuState;
using AnodyneSharp::States::PauseState;
using AnodyneSharp::States::CreditsState;
using AnodyneSharp::States::DeathState;
using AnodyneSharp::States::DialogueState;
using AnodyneSharp::States::IntroState;
using AnodyneSharp::States::CutsceneState;
using AnodyneSharp::States::Substate;
