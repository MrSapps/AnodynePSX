// State implementations
#include "AnodyneSharp/States/AllStates.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Drawing/SpriteDrawer.hpp"
#include "AnodyneSharp/Drawing/DrawingUtilities.hpp"
#include "AnodyneSharp/Drawing/Camera.hpp"
#include "AnodyneSharp/Sounds/SoundManager.hpp"
#include "AnodyneSharp/Entities/EntityManager.hpp"
#include "AnodyneSharp/Entities/Player/Player.hpp"
#include "AnodyneSharp/Entities/Player/Broom.hpp"
#include "AnodyneSharp/Entities/Interactive/AllInteractive.hpp"
#include "AnodyneSharp/Input/KeyInput.hpp"
#include "AnodyneSharp/MapData/Map.hpp"
#include "AnodyneSharp/UI/UITypes.hpp"
#include "AnodyneSharp/Utilities/MapUtilities.hpp"
#include "AnodyneSharp/GameTimes.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/GameEvents/Events.hpp"
#include "AnodyneSharp/Registry/GameConstants.hpp"
#include "AnodyneSharp/Dialogue/DialogueManager.hpp"
#include "SDL3/SDL.h"
#include <algorithm>
#include <fstream>
#include <cmath>

namespace AnodyneSharp::States {

using namespace Entities;
using namespace Drawing;
using namespace Input;
using namespace UI;

using PSState = PlayState::PSState;

static constexpr float SCREEN_W = 160.f;
static constexpr float SCREEN_H = 160.f;
static constexpr float SCROLL_SPEED = 250.f;
static constexpr float PIX_PER_SEC  = 30.f;
static constexpr float TRANSITION_IN = 0.8f;

static bool s_hasBeenReset = false;

PlayState::PlayState() {
    _player = std::make_unique<Player>();

    // Wire up GlobalState callbacks
    GlobalState::SpawnEntity = [this](Entity* e) {
        _entities.push_back(std::unique_ptr<Entity>(e));
        _collision->Register(e);
    };
    GlobalState::SetSubstate = [this](State* s) {
        _childStates.push_back(s);
    };
    GlobalState::FireEvent = [this](GameEvents::GameEvent* e) {
        for (auto& ent : _entities) if (ent->exists) ent->OnEvent(e);
        _player->OnEvent(e);
        delete e;
    };
    GlobalState::DoQuickSave = [this]() { QuickSave(); };
    GlobalState::DoQuickLoad = [this]() { _doQuickLoad = true; };
}

void PlayState::Create() {
    SDL_Log("PlayState::Create NEXT_MAP=%s CURRENT_MAP=%s",
        GlobalState::NEXT_MAP_NAME.c_str(), GlobalState::CURRENT_MAP_NAME.c_str());
    _collision = std::make_unique<CollisionGroups>(0);
    GlobalState::WARP = true;
    Warp();
    SDL_Log("PlayState::Create Warp done");
    _psState = PSState::S_MAP_ENTER;
    SDL_Log("PlayState::Create: psState set");
    GlobalState::pixelation.SetPixelation(10.f);
    SDL_Log("PlayState::Create: pixelation set");
    GlobalState::black_overlay.ForceAlpha(1.f);
    SDL_Log("PlayState::Create: done");
}

void PlayState::Initialize() {}

void PlayState::SpawnEntities() {
    _entities.clear();
    _collision = std::make_unique<CollisionGroups>(0);
    _collision->Register(_player.get());

    Point grid = {GlobalState::CURRENT_GRID_X, GlobalState::CURRENT_GRID_Y};
    auto presets = EntityManager::GetGridEntities(GlobalState::CURRENT_MAP_NAME, grid);
    SDL_Log("SpawnEntities: map=%s grid=(%d,%d) presets=%d",
        GlobalState::CURRENT_MAP_NAME.c_str(), grid.X, grid.Y, (int)presets.size());
    for (auto* preset : presets) {
        if (!preset->GetAlive()) continue;
        SDL_Log("SpawnEntities: creating '%s' at (%.0f,%.0f)", preset->TypeName.c_str(), preset->Position.X, preset->Position.Y);
        auto entity = preset->Create(_player.get());
        SDL_Log("SpawnEntities: created '%s' ptr=%p", preset->TypeName.c_str(), (void*)entity.get());
        if (!entity) continue;
        // Register sub-entities first
        for (auto* sub : entity->SubEntities()) _collision->Register(sub);
        _collision->Register(entity.get());
        _entities.push_back(std::move(entity));
    }
}

void PlayState::Warp() {
    bool mapChanged = (GlobalState::CURRENT_MAP_NAME != GlobalState::NEXT_MAP_NAME);
    SDL_Log("PlayState::Warp mapChanged=%d NEXT=%s CURRENT=%s",
        (int)mapChanged, GlobalState::NEXT_MAP_NAME.c_str(), GlobalState::CURRENT_MAP_NAME.c_str());

    if (mapChanged) {
        GlobalState::events.VisitedMaps.insert(GlobalState::NEXT_MAP_NAME);
        GlobalState::CURRENT_MAP_NAME = GlobalState::NEXT_MAP_NAME;
        SDL_Log("PlayState::Warp: creating Map(%s)", GlobalState::CURRENT_MAP_NAME.c_str());
        _map = std::make_unique<MapData::Map>(GlobalState::CURRENT_MAP_NAME);
        SDL_Log("PlayState::Warp: Map created w=%d h=%d", _map->WidthInTiles, _map->HeightInTiles);
        GlobalState::Map = _map.get();
        GlobalState::MAP_GRID_WIDTH  = _map->WidthInTiles  / 10;
        GlobalState::MAP_GRID_HEIGHT = _map->HeightInTiles / 10;
        GlobalState::RefreshKeyCount = true;
    }

    _player->Position = _player->grid_entrance = GlobalState::PLAYER_WARP_TARGET;
    if (GlobalState::NewMapFacing.has_value()) {
        _player->facing = *GlobalState::NewMapFacing;
        GlobalState::NewMapFacing.reset();
    }

    if (_map) _map->ReloadSettings(_player.get());

    Point gridPos = MapUtilities::GetRoomCoordinate(_player->Position);
    GlobalState::CURRENT_GRID_X = gridPos.X;
    GlobalState::CURRENT_GRID_Y = gridPos.Y;

    UpdateScreenBorders();
    SpawnEntities();

    Vector2 camPos = MapUtilities::GetRoomUpperLeftPos(gridPos);
    SpriteDrawer::Camera_.GoTo(camPos);
}

void PlayState::UpdateScreenBorders() {
    _gridBorders.X = GlobalState::CURRENT_GRID_X * (int)SCREEN_W;
    _gridBorders.Y = GlobalState::CURRENT_GRID_Y * (int)SCREEN_H;
    _gridBorders.Width  = (int)SCREEN_W;
    _gridBorders.Height = (int)SCREEN_H;
    if (GlobalState::CurrentMinimap) GlobalState::CurrentMinimap->Update();
}

void PlayState::DoMapTransition() {
    // Check if player walked off-screen edge
    Point grid = {GlobalState::CURRENT_GRID_X, GlobalState::CURRENT_GRID_Y};
    bool transition = false;
    if (_player->Position.X < _gridBorders.X) {
        grid.X--;
        _player->Position.X = (float)_gridBorders.X - _player->width;
        transition = true;
    } else if (_player->Position.X > _gridBorders.X + _gridBorders.Width - _player->width) {
        grid.X++;
        _player->Position.X = (float)(_gridBorders.X + _gridBorders.Width);
        transition = true;
    } else if (_player->Position.Y < _gridBorders.Y) {
        grid.Y--;
        _player->Position.Y = (float)_gridBorders.Y - _player->height;
        transition = true;
    } else if (_player->Position.Y > _gridBorders.Y + _gridBorders.Height - _player->height) {
        grid.Y++;
        _player->Position.Y = (float)(_gridBorders.Y + _gridBorders.Height);
        transition = true;
    }

    if (transition) {
        SDL_Log("DoMapTransition: grid (%d,%d) player=(%.1f,%.1f) borders=(%d,%d,%d,%d)",
            grid.X, grid.Y,
            _player->Position.X, _player->Position.Y,
            _gridBorders.X, _gridBorders.Y, _gridBorders.Width, _gridBorders.Height);
        _psState = PSState::S_TRANSITION;
        GlobalState::ScreenTransition = true;
        GlobalState::CURRENT_GRID_X = grid.X;
        GlobalState::CURRENT_GRID_Y = grid.Y;
        _player->dontMove = true;
        _player->velocity = {0,0};
        _player->grid_entrance = _player->Position;
        if (_map) { _map->OnTransitionStart(); _map->ReloadSettings(_player->Position); }
        UpdateScreenBorders();
        SpawnEntities();
    }
}

// Check if any interactable entity near player wants interaction
bool PlayState::CheckInteraction() {
    if (_player->state != PlayerState::GROUND || _player->skipBroom) return false;
    Vector2 fv = Entity::FacingDirection(_player->facing);
    for (auto& e : _entities) {
        if (!e->exists) continue;
        auto* ia = dynamic_cast<Interactable*>(e.get());
        if (!ia) continue;
        Rectangle ph = _player->Hitbox();
        Rectangle eh = e->Hitbox();
        // Expand entity hitbox toward player's facing direction
        int ex = (int)std::abs(fv.X), ey = (int)std::abs(fv.Y);
        Rectangle inter = {eh.X - ex, eh.Y - ey, eh.Width + 2*ex, eh.Height + 2*ey};
        if (inter.Intersects(ph)) {
            if (ia->PlayerInteraction(_player->facing)) return true;
        }
    }
    return false;
}

void PlayState::UpdateEntitiesFn() {
    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) && CheckInteraction())
        _player->skipBroom = true;

    for (auto& e : _entities) {
        if (!e->exists) continue;
        e->Update();
        e->PostUpdate();
    }
    _player->Update();
    _player->PostUpdate();
}

// Simple AABB collision between two entities — returns true if overlapping
static bool Overlaps(Entity* a, Entity* b) {
    Rectangle ha = a->Hitbox(), hb = b->Hitbox();
    return ha.X < hb.X+hb.Width  && ha.X+ha.Width  > hb.X &&
           ha.Y < hb.Y+hb.Height && ha.Y+ha.Height > hb.Y;
}

void PlayState::DoCollisions(bool ignorePlayer) {
    // Map collision
    if (_map) {
        if (!ignorePlayer) _map->Collide(_player.get());
        for (auto& e : _entities) if (e->exists) _map->Collide(e.get());
    }

    // Entity vs Entity (player vs world, broom vs world)
    for (auto& e : _entities) {
        if (!e->exists) continue;
        // Player vs entity
        if (!ignorePlayer && Overlaps(_player.get(), e.get())) {
            e->Collided(_player.get());
        }
        // Broom vs entity
        auto* broom = _player->SubEntities().empty() ? nullptr
                    : dynamic_cast<Broom*>(_player->SubEntities()[0]);
        if (broom && broom->exists && Overlaps(broom, e.get())) {
            e->Collided(broom);
        }
        // Keep on screen
        Vector2 tl = MapUtilities::GetRoomUpperLeftPos({GlobalState::CURRENT_GRID_X, GlobalState::CURRENT_GRID_Y});
        Rectangle h = e->Hitbox();
        if (h.X < (int)tl.X)              { e->Position.X = tl.X;              e->touching |= Touching::LEFT; }
        if (h.X+h.Width  > (int)tl.X+160) { e->Position.X = tl.X+160.f-e->width; e->touching |= Touching::RIGHT; }
        if (h.Y < (int)tl.Y)              { e->Position.Y = tl.Y;              e->touching |= Touching::UP; }
        if (h.Y+h.Height > (int)tl.Y+160) { e->Position.Y = tl.Y+160.f-e->height; e->touching |= Touching::DOWN; }
    }
}

void PlayState::Update() {
    // Add newly spawned entities (from GlobalState::SpawnEntity)
    // (Already added directly by the lambda — nothing to do here)

    bool updateEntities = true;
    for (auto* s : _childStates) if (!s->UpdateEntities) { updateEntities = false; break; }

    if (!_childStates.empty()) {
        _player->invincible      = true;
        _player->dontMove        = true;
        _player->actions_disabled = true;

        for (auto it = _childStates.begin(); it != _childStates.end(); ) {
            (*it)->Update();
            if ((*it)->Exit) {
                auto* s = *it;
                if (dynamic_cast<DeathState*>(s)) {
                    Warp();
                    _psState = PSState::S_MAP_ENTER;
                }
                delete s;
                it = _childStates.erase(it);
            } else { ++it; }
        }
        if (_childStates.empty()) {
            _player->dontMove        = false;
            _player->exists          = true;
            _player->actions_disabled = false;
            _player->invincible      = false;
            _player->skipBroom       = true;
        }
        if (updateEntities) DoCollisions(false);
    } else {
        PSState oldState = _psState;
        switch (_psState) {
        case PSState::S_NORMAL:
            if (GlobalState::ToTitle) {
                GlobalState::ToTitle = false;
                GlobalState::GameState->SetState<TitleState>();
                return;
            }
            if (KeyInput::JustPressedRebindableKey(KeyFunctions::Pause) && !GlobalState::disable_menu) {
                _childStates.push_back(new PauseState());
                Sounds::SoundManager::PlaySoundEffect("pause_sound");
            } else if (GlobalState::WARP) {
                _psState = PSState::S_MAP_EXIT;
                if (_map) _map->OnTransitionStart();
            } else {
                DoMapTransition();
            }
            break;

        case PSState::S_TRANSITION: {
            _player->invincible = true;
            Vector2 target = MapUtilities::GetRoomUpperLeftPos({GlobalState::CURRENT_GRID_X, GlobalState::CURRENT_GRID_Y});
            if (SpriteDrawer::Camera_.GoTowards(target, SCROLL_SPEED * GameTimes::DeltaTime())) {
                // Done scrolling
                _player->invincible = false;
                _player->dontMove   = false;
                if (_map) _map->OnTransitionEnd();
                _psState = PSState::S_NORMAL;
                GlobalState::ScreenTransition = false;
                GameTimes::TimeScale = (1.f/60.f) / GameTimes::DeltaTime();
            }
            break;
        }
        case PSState::S_MAP_EXIT:
            _player->invincible      = true;
            _player->dontMove        = true;
            _player->actions_disabled = true;
            GlobalState::pixelation.AddPixelation(PIX_PER_SEC);
            GlobalState::black_overlay.ChangeAlpha(1.f / 0.785f);
            if (GlobalState::black_overlay.alpha >= 1.f || GlobalState::FUCK_IT_MODE_ON) {
                Warp();
                _player->invincible      = true;
                _player->dontMove        = true;
                _player->actions_disabled = true;
                _psState = PSState::S_MAP_ENTER;
                GlobalState::pixelation.SetPixelation(10.f);
            }
            break;

        case PSState::S_MAP_ENTER:
            GlobalState::pixelation.AddPixelation(-PIX_PER_SEC);
            GlobalState::black_overlay.ChangeAlpha(-1.f / TRANSITION_IN);
            if (GlobalState::black_overlay.alpha <= 0.f) {
                _psState = PSState::S_NORMAL;
                _player->dontMove        = false;
                _player->invincible      = false;
                _player->actions_disabled = false;
                GlobalState::WARP = false;
            }
            break;

        default: break;
        }

        bool ignorePl = (oldState == PSState::S_TRANSITION || _psState == PSState::S_TRANSITION);
        DoCollisions(ignorePl);
    }

    if (updateEntities) UpdateEntitiesFn();

    // Trigger dialogue or cutscene if requested
    if (GlobalState::SetDialogueMode) {
        _childStates.push_back(new DialogueState(GlobalState::GetDialogue()));
        GlobalState::SetDialogueMode = false;
    }

    if (_map) _map->Update();

    // Health tracking
    GlobalState::ENEMIES_KILLED = _collision ? _collision->KilledEnemies() : 0;
    if (_childStates.empty() && GlobalState::CUR_HEALTH == 0) {
        Sounds::SoundManager::StopSong();
        _childStates.push_back(new DeathState(_player.get()));
    }

    // Tick the quicksave anti-spam timer
    if (_quickSaveTimer > 0.f) _quickSaveTimer -= GameTimes::DeltaTime();

    // Deferred QuickLoad: must happen at end of update to avoid re-entrancy
    if (_doQuickLoad) { _doQuickLoad = false; QuickLoad(); }
}

void PlayState::QuickSave() {
    if (_quickSaveTimer > 0.f) return;   // prevent spam
    GlobalState::serialized_quicksave = GlobalState::SerializeToString();
    GlobalState::quicksave_checkpoint  = GlobalState::CheckPoint{
        GlobalState::CURRENT_MAP_NAME, _player->Position };
}

void PlayState::QuickLoad() {
    if (GlobalState::serialized_quicksave.empty()) return;
    GlobalState::DeserializeFromString(GlobalState::serialized_quicksave);
    if (GlobalState::quicksave_checkpoint) {
        GlobalState::PLAYER_WARP_TARGET = GlobalState::quicksave_checkpoint->Position;
        GlobalState::NEXT_MAP_NAME      = GlobalState::quicksave_checkpoint->map;
        GlobalState::WARP               = true;
    }
    _quickSaveTimer = 5.f;   // reset anti-spam
}

void PlayState::Draw() {
    if (_map) _map->Draw(SpriteDrawer::Camera_.Bounds());
    for (auto& e : _entities) if (e->exists) e->Draw();
    if (_player->exists) _player->Draw();
    for (auto* s : _childStates) s->Draw();
}

void PlayState::DrawUI() {
    _healthBar.Update();
    _healthBar.Draw();
    for (auto* s : _childStates) s->DrawUI();
}

void PlayState::Reset() { s_hasBeenReset = true; }
bool PlayState::HasBeenReset() { return s_hasBeenReset; }



// ---- DialogueState ----
DialogueState::DialogueState(const std::string& text, bool useMenuBox, bool isIntro)
    : _tb(useMenuBox, isIntro), _speedScale(isIntro ? 4 : 2) {
    _tb.Writer.Text = text;
    _tb.Writer.ResetTextProgress(); // re-syncs AtEndOfText now that Text is set
    _normalSpeed    = _tb.Writer.Speed;
    UpdateEntities  = false;
}

void DialogueState::Update() {
    using namespace Input;
    bool held = KeyInput::IsRebindableKeyPressed(KeyFunctions::Accept) ||
                KeyInput::IsRebindableKeyPressed(KeyFunctions::Cancel);
    bool jp   = KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) ||
                KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel);

    _tb.Writer.Speed = held ? _normalSpeed * _speedScale : _normalSpeed;
    _tb.Update();

    if (_dsState == DSState::WRITING) {
        if (_tb.Writer.AtEndOfText || _tb.Writer.AtEndOfBox) {
            _tb.PauseWriting = true;
            _tb.Writer.Speed = _normalSpeed;
            _dsState = DSState::WAITING;
        }
    } else { // WAITING
        if (jp) {
            Sounds::SoundManager::PlaySoundEffect("dialogue_bloop");
            if (_tb.Writer.AtEndOfText) {
                Exit = true;
            } else {
                // Scroll: remove displayed lines and resume
                int lpb = _tb.Writer.LinesPerBox();
                for (int i = 0; i < lpb; i++) _tb.Writer.RemoveFirstLine();
                _tb.PauseWriting = false;
                _dsState = DSState::WRITING;
            }
        }
    }
}

void DialogueState::DrawUI() { _tb.DrawUI(); }

// ---- DeathState ----
DeathState::DeathState(Entities::Player* player) : _player(player) {
    GlobalState::DeathCount++;
    player->Reset();
    GlobalState::wave.Deactivate();

    _dummy     = std::make_unique<Entities::PlayerDieDummy>(player->Position);
    _deathFade = std::make_unique<Entities::DeathFadeIn>(
        GlobalState::IsCell() ? Color::Black : Color::White);

    Color tc   = GlobalState::IsCell() ? Color::White : Color::Black;
    float x = 50.f, y = 60.f, yStep = 10.f;
    _continueLabel = std::make_unique<UILabel>(Vector2{x,        y         }, false, "Continue?", tc, Drawing::DrawOrder::DEATH_TEXT);
    _yesLabel      = std::make_unique<UILabel>(Vector2{x+19.f,   y+yStep   }, false, "Yes",       tc, Drawing::DrawOrder::DEATH_TEXT);
    _noLabel       = std::make_unique<UILabel>(Vector2{x+17.f,   y+yStep*2.f},false, "No..",      tc, Drawing::DrawOrder::DEATH_TEXT);

    _yesSelected       = true;
    _dState            = DState::FADE_IN;
    UpdateEntities     = false;
    player->dontMove   = true;
    player->exists     = false;
    GlobalState::disable_menu = true;
}

void DeathState::Update() {
    using namespace Input;
    if (_dummy) { _dummy->Update(); _dummy->PostUpdate(); }

    if (!_gotControl && _dummy && _dummy->AnimFinished()) {
        Sounds::SoundManager::PlaySoundEffect("player_hit_1");
        Sounds::SoundManager::PlaySong("gameover");
        _gotControl = true;
        _dState = DState::SELECT;
    }

    if (_deathFade) { _deathFade->Update(); _deathFade->PostUpdate(); }

    if (_dState == DState::FADE_OUT) {
        GlobalState::black_overlay.ChangeAlpha(0.6f);
        if (GlobalState::black_overlay.alpha >= 1.f) {
            GlobalState::WARP = true;
            Exit = true;
        }
    } else if (_dState == DState::SELECT) {
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up)) {
            _yesSelected = true;
            Sounds::SoundManager::PlaySoundEffect("menu_move");
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Down)) {
            _yesSelected = false;
            Sounds::SoundManager::PlaySoundEffect("menu_move");
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
            Sounds::SoundManager::PlaySoundEffect("menu_select");
            GlobalState::CUR_HEALTH = GlobalState::MAX_HEALTH_get();
            _dState = DState::FADE_OUT;
            if (_yesSelected) {
                if (GlobalState::checkpoint.has_value()) {
                    GlobalState::NEXT_MAP_NAME      = GlobalState::checkpoint->map;
                    GlobalState::PLAYER_WARP_TARGET = GlobalState::checkpoint->Position;
                }
                _player->dontMove  = false;
                _player->exists    = true;
                GlobalState::disable_menu = false;
            } else {
                GlobalState::NEXT_MAP_NAME      = "DRAWER";
                GlobalState::PLAYER_WARP_TARGET = {368.f, 224.f};
                _player->ANIM_STATE             = PlayerAnimState::as_slumped;
                GlobalState::inventory.SetEquippedBroom(Entities::BroomType::NONE);
                GlobalState::InDeathRoom        = true;
            }
        }
    }
}

void DeathState::DrawUI() {
    if (_dummy)    _dummy->Draw();
    if (_deathFade) _deathFade->Draw();
    if (_gotControl) {
        if (_continueLabel) _continueLabel->Draw();
        // Draw > marker next to active option
        //Vector2 markerPos = _yesSelected
        //    ? Vector2{_yesLabel->Position.X - 8.f, _yesLabel->Position.Y}
        //    : Vector2{_noLabel->Position.X  - 8.f, _noLabel->Position.Y };
        // Simple text marker
        if (_yesLabel)  _yesLabel->Draw();
        if (_noLabel)   _noLabel->Draw();
    }
}

// ---- CreditsState ----
CreditsState::CreditsState() {
    Sounds::SoundManager::PlaySong("ending");

    float y = 180.f;
    for (int i = 0; i < MAX_LABELS; i++) {
        std::string text = Dialogue::DialogueManager::GetDialogue("misc", "any", "ending", i);
        auto label = std::make_unique<UILabel>(Vector2{0.f, y}, true, text);
        y += 185.f;  // fixed spacing (C# uses max(180, writer height+5))
        _labels.push_back(std::move(label));
    }

    std::string endText = Dialogue::DialogueManager::GetDialogue("misc", "any", "ending", 29);
    _endLabel = std::make_unique<UILabel>(Vector2{0.f, 0.f}, false, endText);
    _endLabel->IsVisible = false;
}

void CreditsState::Update() {
    using namespace Input;

    if (_waitingAccept) {
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept))
            GlobalState::GameState->SetState<TitleState>();
        return;
    }

    if (!_stopScroll) {
        float scale = KeyInput::IsRebindableKeyPressed(KeyFunctions::Accept) ? 8.f : 1.f;
        float speed = SCROLL_SPEED * scale * GameTimes::DeltaTime();

        for (int i = 0; i < (int)_labels.size(); i++) {
            _labels[i]->Position.Y -= speed;
            if (i == (int)_labels.size() - 1 && _labels[i]->Position.Y <= 0.f) {
                _labels[i]->Position.Y = 0.f;
                _stopScroll = true;
            }
        }
    } else {
        // Scrolling done — show end label and wait for Accept
        if (_endLabel) {
            _endLabel->IsVisible = true;
            if (!_labels.empty()) _labels.back()->IsVisible = false;
        }
        _waitingAccept = true;
    }
}

void CreditsState::DrawUI() {
    for (auto& l : _labels) if (l && l->IsVisible) l->Draw();
    if (_endLabel && _endLabel->IsVisible) _endLabel->Draw();
}

// IntroState implemented below with FileSubstate

// ---- TitleState ----
TitleState::TitleState() {}
void TitleState::Create() {
    Sounds::SoundManager::PlaySong("title");
    _background.Load("title_bg", 0.f, -30.f);
    _nexusTex      = Resources::ResourceManager::GetTexture("door");
    _titleTex      = Resources::ResourceManager::GetTexture("title_text");
    _subtitleTex   = Resources::ResourceManager::GetTexture("title_remake");
    _pressEnterTex = Resources::ResourceManager::GetTexture("press_enter");
    GlobalState::flash.Flash(2.f, Color::Black);
    SDL_Log("TitleState::Create door=%p title=%p subtitle=%p enter=%p flash=%.2f",
        (void*)_nexusTex,(void*)_titleTex,(void*)_subtitleTex,(void*)_pressEnterTex,
        GlobalState::flash.GetAlpha());
}
void TitleState::Update() {
    _background.Update();

    if (_pixelating) {
        GlobalState::pixelation.AddPixelation(15.f);
        GlobalState::black_overlay.ChangeAlpha(0.54f);
        if (GlobalState::black_overlay.alpha >= 1.f) {
            GlobalState::pixelation.SetPixelation(0.f);
            GlobalState::black_overlay.alpha = 0.f;
            GlobalState::flash.Deactivate();
            GlobalState::GameState->SetState<MainMenuState>();
        }
        return;
    }

    if (!GlobalState::flash.Active()) {
        _blinkTimer -= GameTimes::DeltaTime();
        if (_blinkTimer <= 0.f) {
            _blinkTimer = 1.f;
            _pressEnterVisible = !_pressEnterVisible;
        }
        using namespace Input;
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel) ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Up)     ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Down)) {
            _pixelating = true;
        }
    }
}
void TitleState::Draw()    {}
void TitleState::DrawUI()  {
    float bgZ   = DrawingUtilities::GetDrawingZ(DrawOrder::BACKGROUND);
    float uiZ   = DrawingUtilities::GetDrawingZ(DrawOrder::UI_OBJECTS);
    float menuZ = DrawingUtilities::GetDrawingZ(DrawOrder::MENUTEXT);

    _background.Draw(bgZ);

    if (_nexusTex) {
        int dy = 180 - _nexusTex->Height;
        Rectangle dst{0, dy, 160, _nexusTex->Height};
        SpriteDrawer::DrawSprite(_nexusTex, dst, nullptr, nullptr, 0.f,
                                 SpriteEffects::None, uiZ);
    }
    if (_titleTex) {
        Rectangle dst{16, 16, _titleTex->Width, _titleTex->Height};
        SpriteDrawer::DrawSprite(_titleTex, dst, nullptr, nullptr, 0.f,
                                 SpriteEffects::None, menuZ);
    }
    if (_subtitleTex) {
        Rectangle dst{45, 47, _subtitleTex->Width, _subtitleTex->Height};
        SpriteDrawer::DrawSprite(_subtitleTex, dst, nullptr, nullptr, 0.f,
                                 SpriteEffects::None, menuZ);
    }
    if (_pressEnterVisible && _pressEnterTex && !GlobalState::flash.Active()) {
        int px = (160 - _pressEnterTex->Width) / 2;
        Rectangle dst{px, 160, _pressEnterTex->Width, _pressEnterTex->Height};
        SpriteDrawer::DrawSprite(_pressEnterTex, dst, nullptr, nullptr, 0.f,
                                 SpriteEffects::None, menuZ);
    }
}

// ---- IntroState ----
IntroState::IntroState() {
    SDL_Log("IntroState::ctor");
    Sounds::SoundManager::PlaySong("blank");
    SDL_Log("IntroState::ctor done");
}

void IntroState::Update() {
    _timer -= GameTimes::DeltaTime();
    switch (_istate) {
    case IState::WaitStart:
        if (_timer <= 0.f) {
            _istate = IState::Writing;
            _timer  = 0.f;
            std::string diag = Dialogue::DialogueManager::GetDialogue("sage","BLANK","intro",0);
            SDL_Log("IntroState: creating DialogueState diag='%s'", diag.c_str());
            _dialogueSub = std::make_unique<DialogueState>(diag, false, true);
            SDL_Log("IntroState: DialogueState created");
        }
        break;
    case IState::Writing:
        if (_dialogueSub) {
            _dialogueSub->Update();
            if (_dialogueSub->Exit) {
                _dialogueSub.reset();
                _istate = IState::WaitEnd;
                _timer  = 1.5f;
            }
        }
        break;
    case IState::WaitEnd:
        if (_timer <= 0.f) {
            Vector2 startPos{77.f, 87.f};
            Registry::GlobalState::PLAYER_WARP_TARGET = startPos;
            Registry::GlobalState::checkpoint = Registry::GlobalState::CheckPoint{"BLANK", startPos};
            Registry::GlobalState::NEXT_MAP_NAME = "BLANK";
            Registry::GlobalState::events.ActivatedNexusPortals.insert("STREET");
            SDL_Log("IntroState: transitioning to PlayState BLANK");
            Registry::GlobalState::GameState->SetState<PlayState>();
        }
        break;
    }
}

void IntroState::DrawUI() {
    if (_dialogueSub) _dialogueSub->DrawUI();
}

// ---- FileSubstate ----
/*static*/ std::string FileSubstate::SavePath(int id) {
    return Registry::GameConstants::SavePath + "Saves/Save_" + std::to_string(id + 1) + ".dat";
}

FileSubstate::FileSubstate(int saveId) : _saveId(saveId) {
    std::ifstream f(SavePath(saveId));
    _saveExists = f.good();

    float x = 70.f, y = 28.f;

    _labels.push_back(std::make_unique<UILabel>(Vector2{x, y},       true,
        _saveExists ? Dialogue::DialogueManager::GetDialogue("misc","any","file",0) : "New Game"));
    _labels.push_back(std::make_unique<UILabel>(Vector2{x, y + 16.f}, true,
        Dialogue::DialogueManager::GetDialogue("misc","any","file",1)));  // "New Game" (clear)
    _labels.push_back(std::make_unique<UILabel>(Vector2{x, y + 32.f}, true,
        Dialogue::DialogueManager::GetDialogue("misc","any","checkpoint",1)));  // Yes
    _labels.push_back(std::make_unique<UILabel>(Vector2{x + 40.f, y + 32.f}, true,
        Dialogue::DialogueManager::GetDialogue("misc","any","checkpoint",2)));  // No

    // Hide confirm labels initially
    if (_labels.size() > 2) _labels[2]->IsVisible = false;
    if (_labels.size() > 3) _labels[3]->IsVisible = false;
    if (!_saveExists && _labels.size() > 1) _labels[1]->IsVisible = false;
}

void FileSubstate::GetControl() {
    Substate::GetControl();
    _fstate = FSState::Game;
    SetSelectorPos();
}

void FileSubstate::Update() {
    if (_loadedSave) {
        // Fade in black then transition
        Registry::GlobalState::black_overlay.ChangeAlpha(0.72f);
        if (Registry::GlobalState::black_overlay.alpha >= 1.f) {
            SDL_Log("FileSubstate: fade done newSave=%d transitioning", (int)_newSave);
            if (_newSave)
                Registry::GlobalState::GameState->SetState<IntroState>();
            else
                Registry::GlobalState::GameState->SetState<PlayState>();
        }
        return;
    }
    if (_lastFState != _fstate) {
        _lastFState = _fstate;
        Sounds::SoundManager::PlaySoundEffect("menu_move");
        SetSelectorPos();
    }
    Substate::Update();
}

void FileSubstate::HandleInput() {
    if (_loadedSave) return;
    using namespace Input;

    switch (_fstate) {
    case FSState::Game:
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
            if (_saveExists)
                ExecuteLoad();
            else
                ExecuteNew();  // no save → new game (goes to IntroState)
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Down) && _saveExists) {
            _fstate = FSState::NewGame;
        } else {
            Substate::HandleInput();
        }
        break;

    case FSState::NewGame:
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
            // Show confirmation
            _confirming = true;
            _fstate = FSState::NoConfirm;
            if (_labels[0]) _labels[0]->IsVisible = false;
            if (_labels[1]) _labels[1]->IsVisible = false;
            if (_labels.size() > 2) _labels[2]->IsVisible = true;
            if (_labels.size() > 3) _labels[3]->IsVisible = true;
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up)) {
            _fstate = FSState::Game;
        } else {
            Substate::HandleInput();
        }
        break;

    case FSState::NoConfirm:
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel)) {
            // Cancel new-game confirmation → back to menu
            _confirming = false;
            _fstate = FSState::NewGame;
            if (_labels[0]) _labels[0]->IsVisible = true;
            if (_labels[1]) _labels[1]->IsVisible = true;
            if (_labels.size() > 2) _labels[2]->IsVisible = false;
            if (_labels.size() > 3) _labels[3]->IsVisible = false;
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Left)) {
            _fstate = FSState::YesConfirm;
        }
        break;

    case FSState::YesConfirm:
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
            ExecuteNew();
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel)) {
            _fstate = FSState::Game;
            _confirming = false;
            if (_labels[0]) _labels[0]->IsVisible = true;
            if (_labels[1]) _labels[1]->IsVisible = true;
            if (_labels.size() > 2) _labels[2]->IsVisible = false;
            if (_labels.size() > 3) _labels[3]->IsVisible = false;
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Right)) {
            _fstate = FSState::NoConfirm;
        }
        break;
    }
}

void FileSubstate::DrawUI() {
    for (auto& l : _labels) if (l && l->IsVisible) l->Draw();
    Substate::DrawUI();
}

void FileSubstate::SetSelectorPos() {
    Vector2 pos{70.f, 28.f};
    switch (_fstate) {
    case FSState::Game:       pos.Y = 28.f;  break;
    case FSState::NewGame:    pos.Y = 44.f;  break;
    case FSState::YesConfirm: pos.Y = 60.f;  break;
    case FSState::NoConfirm:  pos = {110.f, 60.f}; break;
    }
    _selector.Position = pos - Vector2{(float)_selector.width, -2.f};
}

void FileSubstate::ExecuteLoad() {
    Registry::GlobalState::CurrentSaveGame = std::to_string(_saveId + 1);
    Registry::GlobalState::ResetValues();
    Registry::GlobalState::LoadSave(std::to_string(_saveId + 1));
    Sounds::SoundManager::PlaySoundEffect("menu_select");
    _loadedSave = true;
    _newSave    = false;
}

void FileSubstate::ExecuteNew() {
    Registry::GlobalState::CurrentSaveGame = std::to_string(_saveId + 1);
    Registry::GlobalState::ResetValues();
    Sounds::SoundManager::PlaySoundEffect("menu_select");
    _loadedSave = true;
    _newSave    = true;
}

// ---- MainMenuState ----
int MainMenuState::_state = 0;

MainMenuState::MainMenuState() {
    UpdateEntities = false;
    Sounds::SoundManager::PlaySong("title");

    float x = 10.f, y = (float)Registry::GameConstants::HEADER_HEIGHT
                       - (float)Registry::GameConstants::LineOffset() + 11.f;
    float step = (float)(Registry::GameConstants::FONT_LINE_HEIGHT()
                       - Registry::GameConstants::LineOffset()) * 2.f;

    _labels.push_back(std::make_unique<UILabel>(Vector2{x, y},              false, "1"));
    _labels.push_back(std::make_unique<UILabel>(Vector2{x, y + step},       false, "2"));
    _labels.push_back(std::make_unique<UILabel>(Vector2{x, y + step * 2.f}, false, "3"));
    std::string cfgTxt = Dialogue::DialogueManager::GetDialogue("misc","any","config",0);
    if (cfgTxt == "No text available.") cfgTxt = "Config";
    _labels.push_back(std::make_unique<UILabel>(Vector2{x, y + step * 3.f}, false, cfgTxt));

    _lastState = _state;
    StateChanged();
}

std::unique_ptr<Substate> MainMenuState::CreateSubstate(int idx) {
    if (idx >= 0 && idx <= 2) return std::make_unique<FileSubstate>(idx);
    if (idx == 3) return std::make_unique<ConfigSubstate>();
    return std::make_unique<Substate>();
}

void MainMenuState::StateChanged() {
    _lastState = _state;
    _selector.Position = Vector2{2.f, 34.f + (float)_state * 16.f};
    _substate = CreateSubstate(_state);
}

void MainMenuState::Update() {
    using namespace Input;

    if (_substate) _substate->Update();

    if (!_inSubstate && KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel)) {
        Sounds::SoundManager::PlaySoundEffect("pause_sound");
        GlobalState::GameState->SetState<TitleState>();
        return;
    } else if (!_inSubstate) {
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up) && _state > 0) {
            _state--;
            Sounds::SoundManager::PlaySoundEffect("menu_move");
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Down) && _state < 3) {
            _state++;
            Sounds::SoundManager::PlaySoundEffect("menu_move");
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) ||
                   KeyInput::JustPressedRebindableKey(KeyFunctions::Right)) {
            Sounds::SoundManager::PlaySoundEffect("menu_select");
            _inSubstate = true;
            if (_substate) _substate->GetControl();
        }
    } else {
        if (_substate) _substate->HandleInput();
        if (_substate && _substate->Exit) {
            _inSubstate = false;
            _substate->Exit = false;
        }
    }

    if (_lastState != _state) StateChanged();

    _selector.Update();
    _selector.PostUpdate();
}

void MainMenuState::DrawUI() {
    for (auto& l : _labels) if (l) l->Draw();
    _selector.Draw();
    if (_substate) _substate->DrawUI();
}

// ============================================================
// Substate base class
// ============================================================
Substate::Substate() {
    _selector.visible = false; // hidden until GetControl() is called (matches C# object initializer)
}

void Substate::HandleInput() {
    using namespace Input;
    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel) ||
        KeyInput::JustPressedRebindableKey(KeyFunctions::Left)) {
        ExitSubState();
    }
}
void Substate::GetControl() {
    _selector.visible = true;
}
void Substate::Update() {
    _selector.Update();
    _selector.PostUpdate();
}
void Substate::DrawUI() {
    _selector.Draw();
}
void Substate::ExitSubState() {
    Exit = true;
    _selector.visible = false;
    Sounds::SoundManager::PlaySoundEffect("menu_select");
    OnExit();
}

// ============================================================
// DialogueSubstate
// ============================================================
void DialogueSubstate::HandleInput() {
    if (!InDialogueMode()) Substate::HandleInput();
}
void DialogueSubstate::Update() {
    Substate::Update();
    if (_dialogueSub) {
        _dialogueSub->Update();
        if (_dialogueSub->Exit) _dialogueSub.reset();
    }
}
void DialogueSubstate::DrawUI() {
    Substate::DrawUI();
    if (_dialogueSub) _dialogueSub->DrawUI();
}
void DialogueSubstate::SetDialogue(const std::string& text) {
    _dialogueSub = std::make_unique<DialogueState>(text, true);
}

// ============================================================
// SaveSubstate
// ============================================================
SaveSubstate::SaveSubstate() {
    using DM = Dialogue::DialogueManager;
    float x = 69.f, y = 28.f - (float)Registry::GameConstants::LineOffset();
    float step = (float)Registry::GameConstants::FONT_LINE_HEIGHT();

    auto lbl = [&](const std::string& txt) {
        auto l = std::make_unique<UILabel>(Vector2{x, y}, true, txt);
        y += step * 2.f - 4.f;
        return l;
    };

    _labels.push_back(lbl(DM::GetDialogue("misc","any","controls",15)));  // QuickSave
    _labels.push_back(lbl(DM::GetDialogue("misc","any","controls",16)));  // QuickLoad
    _labels.push_back(lbl(DM::GetDialogue("misc","any","save",0)));       // Save
    _labels.push_back(lbl(DM::GetDialogue("misc","any","save",3)));       // Save+Title
    _labels.push_back(lbl(DM::GetDialogue("misc","any","save",5)));       // Save+Quit
    _labels.push_back(lbl(DM::GetDialogue("misc","any","save",6)));       // Quit

    // Deaths label (below quit, extra gap)
    y += step * 2.f;
    std::string deathStr = DM::GetDialogue("misc","any","save",7)
                         + std::to_string(Registry::GlobalState::DeathCount);
    _labels.push_back(std::make_unique<UILabel>(Vector2{x, y}, true, deathStr));
}

void SaveSubstate::GetControl() {
    Substate::GetControl();
    _curAction = SaveAction::QuickSave;
    SetSelectorPos();
}

void SaveSubstate::Update() {
    if (_lastAction != _curAction) {
        _lastAction = _curAction;
        Sounds::SoundManager::PlaySoundEffect("menu_move");
        SetSelectorPos();
    }
    Substate::Update();
}

void SaveSubstate::HandleInput() {
    using namespace Input;
    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up)) {
        if (_curAction > SaveAction::QuickSave)
            _curAction = static_cast<SaveAction>(static_cast<int>(_curAction) - 1);
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Down)) {
        if (_curAction < SaveAction::Quit)
            _curAction = static_cast<SaveAction>(static_cast<int>(_curAction) + 1);
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
        bool playSound = true;
        switch (_curAction) {
        case SaveAction::QuickSave:
            if (Registry::GlobalState::DoQuickSave) Registry::GlobalState::DoQuickSave();
            playSound = false;
            if (_labels.size() > 0) _labels[0]->SetText(Dialogue::DialogueManager::GetDialogue("misc","any","save",1));
            break;
        case SaveAction::QuickLoad:
            if (Registry::GlobalState::DoQuickLoad) Registry::GlobalState::DoQuickLoad();
            playSound = false;
            break;
        case SaveAction::Save:
            Registry::GlobalState::SaveGame();
            if (_labels.size() > 2) _labels[2]->SetText(Dialogue::DialogueManager::GetDialogue("misc","any","save",1));
            break;
        case SaveAction::SaveTitle:
            Registry::GlobalState::SaveGame();
            Registry::GlobalState::GameState->SetState<TitleState>();
            break;
        case SaveAction::SaveQuit:
            Registry::GlobalState::SaveGame();
            Registry::GlobalState::ClosingGame = true;
            break;
        case SaveAction::Quit:
            Registry::GlobalState::ClosingGame = true;
            break;
        default: break;
        }
        if (playSound) Sounds::SoundManager::PlaySoundEffect("menu_select");
    } else {
        Substate::HandleInput();
    }
}

void SaveSubstate::DrawUI() {
    for (auto& l : _labels) if (l) l->Draw();
    Substate::DrawUI();
}

void SaveSubstate::SetSelectorPos() {
    int idx = static_cast<int>(_curAction);
    if (idx < (int)_labels.size() && _labels[idx])
        _selector.Position = _labels[idx]->Position - Vector2{(float)_selector.width, -2.f};
}

// ============================================================
// MapSubstate
// ============================================================
MapSubstate::MapSubstate() {
    using DM = Dialogue::DialogueManager;
    float x = 73.f;
    float y = 144.f - (float)Registry::GameConstants::LineOffset();

    _noMapLabel  = std::make_unique<UILabel>(Vector2{x + 14.f, (float)Registry::GameConstants::HEADER_HEIGHT + 11.f},
                                             true, DM::GetDialogue("misc","any","map",3));

    bool isDungeon = Registry::GlobalState::IsDungeon();
    _returnLabel = std::make_unique<UILabel>(
        Vector2{x, y - (float)Registry::GameConstants::FONT_LINE_HEIGHT() * 2.f},
        true, DM::GetDialogue("misc","any","map", isDungeon ? 5 : 4));
    _returnLabel->IsVisible = Registry::GlobalState::ReturnTarget.has_value();

    y += 4.f;
    _yesLabel = std::make_unique<UILabel>(Vector2{x,        y}, true, DM::GetDialogue("misc","any","checkpoint",1));
    _noLabel  = std::make_unique<UILabel>(Vector2{x + 56.f, y}, true, DM::GetDialogue("misc","any","checkpoint",2));
    _mapSheet = Drawing::Spritesheet::Spritesheet(ResourceManager::GetTexHandle("minimap_tiles", true), 7, 7);
}

void MapSubstate::GetControl() {
    if (!Registry::GlobalState::ReturnTarget.has_value()) { ExitSubState(); return; }
    Substate::GetControl();
    _nav = MapNav::Return;
    SetSelectorPos();
}

void MapSubstate::Update() {
    if (_lastNav != _nav) { _lastNav = _nav; SetSelectorPos(); }
    _blinkTimer += GameTimes::DeltaTime();
    constexpr float BLINK = 0.4f;
    if (_blinkTimer > 2.f * BLINK) _blinkTimer -= 2.f * BLINK;
    Substate::Update();
}

void MapSubstate::HandleInput() {
    using namespace Input;
    switch (_nav) {
    case MapNav::Return:
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
            _nav = MapNav::No;
            Sounds::SoundManager::PlaySoundEffect("menu_select");
        } else { Substate::HandleInput(); }
        break;
    case MapNav::No:
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) ||
            KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel)) {
            _nav = MapNav::Return;
            Sounds::SoundManager::PlaySoundEffect("menu_select");
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Left)) {
            _nav = MapNav::Yes;
            Sounds::SoundManager::PlaySoundEffect("menu_move");
        }
        break;
    case MapNav::Yes:
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
            Sounds::SoundManager::PlaySoundEffect("menu_select");
            if (Registry::GlobalState::ReturnTarget.has_value())
                Registry::GlobalState::ReturnTarget->Warp(Vector2{10.f, 34.f});
            ExitSubState();
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel)) {
            _nav = MapNav::Return;
            Sounds::SoundManager::PlaySoundEffect("menu_select");
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Right)) {
            _nav = MapNav::No;
            Sounds::SoundManager::PlaySoundEffect("menu_move");
        }
        break;
    }
}

void MapSubstate::DrawUI() {
    if (_returnLabel) _returnLabel->Draw();
    if (_nav != MapNav::Return) {
        if (_yesLabel) _yesLabel->Draw();
        if (_noLabel)  _noLabel->Draw();
    }
    Minimap* minimap = Registry::GlobalState::CurrentMinimap;
    if (minimap) {
        if (minimap->tiles.Width == 0) {
            if (_noMapLabel) _noMapLabel->Draw();
        } else {
            int mx = 110 - minimap->tiles.Width  * _mapSheet.Width  / 2;
            int my = 70  - minimap->tiles.Height * _mapSheet.Height / 2;
            minimap->Draw(_mapSheet, Vector2{(float)mx, (float)my},
                          nullptr, _blinkTimer < _playerBlink);
        }
    }
    Substate::DrawUI();
}

void MapSubstate::SetSelectorPos() {
    UILabel* target = nullptr;
    switch (_nav) {
    case MapNav::Return: target = _returnLabel.get(); break;
    case MapNav::No:     target = _noLabel.get();     break;
    case MapNav::Yes:    target = _yesLabel.get();    break;
    }
    if (target) _selector.Position = target->Position - Vector2{(float)_selector.width, -2.f};
}

// ============================================================
// EquipSubstate
// ============================================================
EquipSubstate::EquipSubstate() {
    using DM = Dialogue::DialogueManager;
    auto& inv = Registry::GlobalState::inventory;
    float x = 65.f, y = 25.f, step = 24.f;

    auto broomLbl = [&](bool has, int idx) {
        static const int dlgIdx[] = {1, 3, 4, 2};
        std::string txt = has ? DM::GetDialogue("misc","any","items", dlgIdx[idx]) : "-";
        auto l = std::make_unique<UILabel>(Vector2{x, y + step * idx}, true, txt);
        return l;
    };
    _broomLabels.push_back(broomLbl(inv.HasBroom,       0));
    _broomLabels.push_back(broomLbl(inv.HasLengthen,    1));
    _broomLabels.push_back(broomLbl(inv.HasWiden,       2));
    _broomLabels.push_back(broomLbl(inv.HasTransformer, 3));

    // Build bottom row
    if (inv.CanJump)                              _bottomRow.push_back(EquipItem::Shoes);
    if (inv.tradeState != InventoryManager::TradeState::NONE) _bottomRow.push_back(EquipItem::TradeItem);
    _bottomRow.push_back(EquipItem::Key1);
    _bottomRow.push_back(EquipItem::Key2);
    _bottomRow.push_back(EquipItem::Key3);
}

void EquipSubstate::GetControl() {
    if (!Registry::GlobalState::inventory.HasAnyBroom()) { Exit = true; return; }
    DialogueSubstate::GetControl();
    _state = EquipItem::Broom;
    _last  = _state;
    SetSelectorPos();
}

void EquipSubstate::Update() {
    if (_last != _state) {
        _last = _state;
        SetSelectorPos();
        Sounds::SoundManager::PlaySoundEffect("menu_move");
    }
    DialogueSubstate::Update();
}

void EquipSubstate::HandleInput() {
    if (InDialogueMode()) return;
    using namespace Input;
    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up)) {
        if (_state == EquipItem::Broom) return;
        if (_state >= EquipItem::Shoes) { _state = EquipItem::Transformer; return; }
        _state = static_cast<EquipItem>(static_cast<int>(_state) - 1);
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Down)) {
        if (_state >= EquipItem::TradeItem) return;
        if (_state == EquipItem::Transformer) {
            if (_bottomRow.empty()) return;
            _state = _bottomRow[0]; _bottomIdx = 0;
        } else {
            _state = static_cast<EquipItem>(static_cast<int>(_state) + 1);
        }
    } else if (_state >= EquipItem::Shoes &&
               KeyInput::JustPressedRebindableKey(KeyFunctions::Right)) {
        if (_bottomIdx < (int)_bottomRow.size() - 1)
            _state = _bottomRow[++_bottomIdx];
    } else if (_state >= EquipItem::Shoes && _bottomIdx > 0 &&
               KeyInput::JustPressedRebindableKey(KeyFunctions::Left)) {
        _state = _bottomRow[--_bottomIdx];
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
        UseItem();
    } else {
        Substate::HandleInput();
    }
}

void EquipSubstate::UseItem() {
    using DM = Dialogue::DialogueManager;
    auto& inv = Registry::GlobalState::inventory;
    switch (_state) {
    case EquipItem::Broom:       EquipBroom(Entities::BroomType::Normal);      break;
    case EquipItem::Extend:      EquipBroom(Entities::BroomType::Long);        break;
    case EquipItem::Widen:       EquipBroom(Entities::BroomType::Wide);        break;
    case EquipItem::Transformer: EquipBroom(Entities::BroomType::Transformer); break;
    case EquipItem::Shoes:
        SetDialogue(DM::GetDialogue("misc","any","items",5)); break;
    case EquipItem::TradeItem:
        SetDialogue(DM::GetDialogue("misc","any","items",
            inv.tradeState == InventoryManager::TradeState::SHOES ? 6 : 7)); break;
    case EquipItem::Key1: if (inv.BigKeyStatus[0]) SetDialogue(DM::GetDialogue("misc","any","items",8)); break;
    case EquipItem::Key2: if (inv.BigKeyStatus[1]) SetDialogue(DM::GetDialogue("misc","any","items",9)); break;
    case EquipItem::Key3: if (inv.BigKeyStatus[2]) SetDialogue(DM::GetDialogue("misc","any","items",10)); break;
    }
}

void EquipSubstate::EquipBroom(Entities::BroomType bt) {
    Registry::GlobalState::inventory.SetEquippedBroom(bt);
    ExitSubState();
}

void EquipSubstate::DrawUI() {
    for (auto& l : _broomLabels) if (l) l->Draw();
    DialogueSubstate::DrawUI();
}

void EquipSubstate::SetSelectorPos() {
    Vector2 pos{65.f, 25.f};
    switch (_state) {
    case EquipItem::Broom:       pos.Y = 25.f;      break;
    case EquipItem::Extend:      pos.Y = 49.f;      break;
    case EquipItem::Widen:       pos.Y = 73.f;      break;
    case EquipItem::Transformer: pos.Y = 97.f;      break;
    case EquipItem::Shoes:       pos = {62.f,130.f}; break;
    case EquipItem::TradeItem:   pos = {78.f,130.f}; break;
    case EquipItem::Key1:        pos = {95.f,130.f}; break;
    case EquipItem::Key2:        pos = {111.f,130.f};break;
    case EquipItem::Key3:        pos = {127.f,130.f};break;
    }
    _selector.Position = pos - Vector2{(float)_selector.width, -2.f};
}

// ============================================================
// CardSubstate
// ============================================================
CardSubstate::CardSubstate() {
    using DM = Dialogue::DialogueManager;
    auto& inv = Registry::GlobalState::inventory;
    std::string countTxt = std::to_string(inv.CardCount()) + " "
                         + DM::GetDialogue("misc","any","cards",1);
    _cardsLabel = std::make_unique<UILabel>(
        Vector2{70.f, 146.f - (float)Registry::GameConstants::LineOffset()}, true, countTxt);
    _pageLabel  = std::make_unique<UILabel>(Vector2{91.f, 156.f}, true, "1/4");
}

void CardSubstate::GetControl() {
    DialogueSubstate::GetControl();
    _page = 0; _selected = 0;
    SetCardPage();
}

void CardSubstate::Update() {
    DialogueSubstate::Update();
}

void CardSubstate::HandleInput() {
    if (InDialogueMode()) return;
    using namespace Input;
    bool moved = false;
    int& sel = _selected;

    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel)) {
        if (sel < 0) { sel = -sel; }
        ExitSubState(); return;
    }

    if (sel < 0) {
        // Page selector row
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up)) {
            sel = -sel;
            Sounds::SoundManager::PlaySoundEffect("menu_select");
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Left) && _page > 0) {
            _page--; SetCardPage(); moved = true;
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Right) && _page < 3) {
            _page++; SetCardPage(); moved = true;
        }
        if (moved) Sounds::SoundManager::PlaySoundEffect("menu_move");
        return;
    }

    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Right)) {
        if (sel % 3 == 2) {
            if (_page < 3) { sel -= 2; _page++; SetCardPage(); }
        } else { sel++; }
        moved = true;
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Left)) {
        if (sel % 3 == 0) {
            if (_page == 0) { ExitSubState(); return; }
            sel += 2; _page--; SetCardPage();
        } else { sel--; }
        moved = true;
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up)) {
        if (sel < 3) return;
        sel -= 3; moved = true;
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Down)) {
        if (sel > 8) {
            // Enter page selector
            sel = -sel;
            Sounds::SoundManager::PlaySoundEffect("menu_select"); return;
        }
        sel += 3; moved = true;
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
        int cardID = _page * 12 + sel;
        if (cardID < 49 && Registry::GlobalState::inventory.CardStatus[cardID])
            SetDialogue(Dialogue::DialogueManager::GetDialogue("card","ETC","one",cardID));
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::NextPage) && _page < 3) {
        _page++; SetCardPage(); moved = true;
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::PreviousPage) && _page > 0) {
        _page--; SetCardPage(); moved = true;
    }

    if (moved) Sounds::SoundManager::PlaySoundEffect("menu_move");
}

void CardSubstate::DrawUI() {
    if (_cardsLabel) _cardsLabel->Draw();
    if (_pageLabel)  _pageLabel->Draw();
    DialogueSubstate::DrawUI();
}

void CardSubstate::SetCardPage() {
    if (_pageLabel) {
        std::string p = std::to_string(_page + 1) + "/4";
        _pageLabel->SetText(p);
    }
}

// ============================================================
// ConfigSubstate
// ============================================================
ConfigSubstate::ConfigSubstate() {
    //using DM = Dialogue::DialogueManager;
    float x = 69.f, y = 28.f - (float)Registry::GameConstants::LineOffset();
    float step = (float)Registry::GameConstants::FONT_LINE_HEIGHT();

    auto mkLbl = [&](const std::string& txt) {
        auto l = std::make_unique<UILabel>(Vector2{x, y}, true, txt);
        y += step * 2.f;
        return l;
    };

    auto& cfg = Registry::GlobalState::settings;
    _labels.push_back(mkLbl("BGM: " + std::to_string((int)(cfg.music_volume_scale * 100)) + "%"));
    _labels.push_back(mkLbl("SFX: " + std::to_string((int)(cfg.sfx_volume_scale   * 100)) + "%"));

    // Language label
    static const char* langNames[] = { "EN","ES","IT","JP","KR","PT-BR","ZH-CN" };
    int li = (int)cfg.language;
    _labels.push_back(mkLbl(std::string("Lang: ")
        + langNames[li < 7 ? li : 0]));
}

void ConfigSubstate::GetControl() {
    Substate::GetControl();
    _cur = ConfigItem::MusicVol;
    SetSelectorPos();
}

void ConfigSubstate::Update() {
    if (_last != _cur) {
        _last = _cur;
        SetSelectorPos();
        Sounds::SoundManager::PlaySoundEffect("menu_move");
    }
    Substate::Update();
}

void ConfigSubstate::HandleInput() {
    using namespace Input;
    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up)) {
        if (_cur > ConfigItem::MusicVol)
            _cur = static_cast<ConfigItem>(static_cast<int>(_cur) - 1);
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Down)) {
        if (_cur < ConfigItem::Language)
            _cur = static_cast<ConfigItem>(static_cast<int>(_cur) + 1);
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Right) ||
               KeyInput::JustPressedRebindableKey(KeyFunctions::Accept)) {
        auto& cfg = Registry::GlobalState::settings;
        if (_cur == ConfigItem::MusicVol) {
            cfg.music_volume_scale = std::min(1.f, cfg.music_volume_scale + 0.1f);
            Sounds::SoundManager::SetSongVolume(Sounds::SoundManager::GetVolume());
            _labels[0]->SetText("BGM: " + std::to_string((int)(cfg.music_volume_scale * 100)) + "%");
        } else if (_cur == ConfigItem::SFXVol) {
            cfg.sfx_volume_scale = std::min(1.f, cfg.sfx_volume_scale + 0.1f);
            _labels[1]->SetText("SFX: " + std::to_string((int)(cfg.sfx_volume_scale * 100)) + "%");
        } else if (_cur == ConfigItem::Language) {
            int li = (int)cfg.language;
            li = (li + 1) % 7;
            cfg.language = static_cast<Language>(li);
            Dialogue::DialogueManager::SetLanguage(cfg.language);
            static const char* langNames[] = { "EN","ES","IT","JP","KR","PT-BR","ZH-CN" };
            _labels[2]->SetText(std::string("Lang: ") + langNames[li]);
        }
        Sounds::SoundManager::PlaySoundEffect("menu_select");
    } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Left)) {
        auto& cfg = Registry::GlobalState::settings;
        if (_cur == ConfigItem::MusicVol) {
            cfg.music_volume_scale = std::max(0.f, cfg.music_volume_scale - 0.1f);
            Sounds::SoundManager::SetSongVolume(Sounds::SoundManager::GetVolume());
            _labels[0]->SetText("BGM: " + std::to_string((int)(cfg.music_volume_scale * 100)) + "%");
        } else if (_cur == ConfigItem::SFXVol) {
            cfg.sfx_volume_scale = std::max(0.f, cfg.sfx_volume_scale - 0.1f);
            _labels[1]->SetText("SFX: " + std::to_string((int)(cfg.sfx_volume_scale * 100)) + "%");
        } else if (_cur == ConfigItem::Language) {
            int li = (int)cfg.language;
            li = (li + 6) % 7;
            cfg.language = static_cast<Language>(li);
            Dialogue::DialogueManager::SetLanguage(cfg.language);
            static const char* langNames[] = { "EN","ES","IT","JP","KR","PT-BR","ZH-CN" };
            _labels[2]->SetText(std::string("Lang: ") + langNames[li]);
        }
        Sounds::SoundManager::PlaySoundEffect("menu_select");
    } else {
        Substate::HandleInput();
    }
}

void ConfigSubstate::DrawUI() {
    for (auto& l : _labels) if (l) l->Draw();
    Substate::DrawUI();
}

void ConfigSubstate::SetSelectorPos() {
    int idx = static_cast<int>(_cur);
    if (idx < (int)_labels.size() && _labels[idx])
        _selector.Position = _labels[idx]->Position - Vector2{(float)_selector.width, -2.f};
}

void ConfigSubstate::OnExit() {
    Registry::GlobalState::settings.Save();
}

// ============================================================
// PauseState
// ============================================================
int PauseState::_menuState = 0;

PauseState::PauseState() {
    UpdateEntities = false;
    using DM = Dialogue::DialogueManager;
    const char* keys[] = { "map", "items", "cards", "save", "config" };
    const char* fb[]   = { "Map", "Items", "Cards", "Save", "Config" };
    float x = 10.f, y = (float)Registry::GameConstants::HEADER_HEIGHT + 11.f;

    for (int i = 0; i < MENU_COUNT; ++i) {
        std::string txt = DM::GetDialogue("misc","any",keys[i],0);
        if (txt == "No text available.") txt = fb[i];
        _menuLabels.push_back(std::make_unique<UILabel>(Vector2{x, y + 16.f * i}, true, txt));
    }
    _playtimeLabel = std::make_unique<UILabel>(Vector2{1.f, 154.f}, true, "00:00:00");
    _lastState = _menuState;
    StateChanged();
}

PauseState::~PauseState() = default;

std::unique_ptr<Substate> PauseState::CreateSubstate(int idx) {
    switch (idx) {
    case 0: return std::make_unique<MapSubstate>();
    case 1: return std::make_unique<EquipSubstate>();
    case 2: return std::make_unique<CardSubstate>();
    case 3: return std::make_unique<SaveSubstate>();
    case 4: return std::make_unique<ConfigSubstate>();
    case 5: return std::make_unique<CheatzSubstate>();
    default: return std::make_unique<Substate>();
    }
}

void PauseState::StateChanged() {
    _lastState = _menuState;
    _substate  = CreateSubstate(_menuState);
}

void PauseState::Update() {
    using namespace Input;
    if (Registry::GlobalState::CUR_HEALTH == 0) { Exit = true; return; }

    if (Registry::GlobalState::RefreshLabels) {
        Registry::GlobalState::RefreshLabels = false;
        // refresh broom labels etc. would go here
    }

    // Update playtime
    auto ms = Registry::GlobalState::PlayTime();
    long long totalSec = ms.count() / 1000;
    int h = (int)(totalSec / 3600), m = (int)((totalSec % 3600) / 60), s = (int)(totalSec % 60);
    char buf[16]; std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
    if (_playtimeLabel) _playtimeLabel->SetText(buf);

    // Always update substate
    if (_substate) _substate->Update();

    if (KeyInput::JustPressedRebindableKey(KeyFunctions::Pause) ||
        (!_inSubstate && KeyInput::JustPressedRebindableKey(KeyFunctions::Cancel))) {
        Sounds::SoundManager::PlaySoundEffect("pause_sound");
        Exit = true;
        return;
    }

    if (_inSubstate) {
        if (_substate) _substate->HandleInput();
        if (_substate && _substate->Exit) {
            _inSubstate = false;
            _substate->Exit = false;
            if (Registry::GlobalState::WARP) { Exit = true; return; }
        }
    } else {
        // Browse main menu
        if (KeyInput::JustPressedRebindableKey(KeyFunctions::Up)) {
            _cheatCounter = 0;
            if (_menuState > 0) { _menuState--; Sounds::SoundManager::PlaySoundEffect("menu_move"); }
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Down)) {
            bool inCheat = (_menuState == MENU_COUNT);
            if (!inCheat) {
                if (_menuState == MENU_COUNT - 1) {
                    Sounds::SoundManager::PlaySoundEffect("menu_move");
                    if (++_cheatCounter >= CHEAT_MAX) {
                        _cheatCounter = 0;
                        _menuState++;
                    }
                } else {
                    _menuState++;
                    Sounds::SoundManager::PlaySoundEffect("menu_move");
                }
            }
        } else if (KeyInput::JustPressedRebindableKey(KeyFunctions::Accept) ||
                   KeyInput::JustPressedRebindableKey(KeyFunctions::Right)) {
            Sounds::SoundManager::PlaySoundEffect("menu_select");
            _inSubstate = true;
            if (_substate) _substate->GetControl();
        }

        if (_lastState != _menuState) StateChanged();
    }

    if (Registry::GlobalState::WARP) { Exit = true; }
}

void PauseState::DrawUI() {
    for (auto& l : _menuLabels) if (l) l->Draw();
    if (_playtimeLabel) _playtimeLabel->Draw();
    if (_substate) _substate->DrawUI();
}

void PauseState::Reset() { _menuState = 0; }

// ---- CutsceneState ----
CutsceneState::CutsceneState(Coroutine /*stateCoroutine*/) {
    _oldDarkness = GlobalState::darkness.Alpha;
}

void CutsceneState::Update() {
    // Update all active entities
    for (auto* e : _entities) {
        if (e && e->exists) { e->Update(); e->PostUpdate(); }
    }
}

void CutsceneState::Draw() {
    if (_map) _map->Draw(SpriteDrawer::Camera_.Bounds());
    for (auto* e : _entities) {
        if (e && e->exists) e->Draw();
    }
}

void CutsceneState::DrawUI() {}

void CutsceneState::Return() {
    SpriteDrawer::Camera_.GoTo(
        MapUtilities::GetRoomUpperLeftPos(GlobalState::CurrentMapGrid()));
    if (_map) _map->ReloadSettings(SpriteDrawer::Camera_.Position2D(), true, nullptr, false);
    GlobalState::darkness.ForceAlpha(_oldDarkness);
    DrawPlayState  = true;
    UpdateEntities = true;
    _map = nullptr;
}

void CutsceneState::Warp(const std::string& map, const Point& grid) {
    _map = new MapData::Map(map);
    SpriteDrawer::Camera_.GoTo(Vector2{(float)(grid.X * 160), (float)(grid.Y * 160)});
    DrawPlayState  = false;
    UpdateEntities = false;
    _map->ReloadSettings(SpriteDrawer::Camera_.Position2D(), true, nullptr, false);
}

} // namespace AnodyneSharp::States

