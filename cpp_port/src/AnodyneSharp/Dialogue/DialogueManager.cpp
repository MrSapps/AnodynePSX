#include "AnodyneSharp/Dialogue/DialogueManager.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"
#include "AnodyneSharp/Resources/ResourceManager.hpp"
#include "AnodyneSharp/Input/KeyInput.hpp"
#include "SDL3/SDL.h"
#include <fstream>

namespace AnodyneSharp::Dialogue {

Language DialogueManager::CurrentLanguage = Language::EN;
std::unordered_map<std::string, DialogueNPC> DialogueManager::SceneTree;

// Language enum → file suffix
static const char* LangSuffix(Language lang) {
    switch (lang) {
    case Language::ES:    return "ES";
    case Language::IT:    return "IT";
    case Language::JP:    return "JP";
    case Language::KR:    return "KR";
    case Language::PT_BR: return "PT_BR";
    case Language::ZH_CN: return "ZH_CN";
    default:              return "EN";
    }
}

void DialogueManager::Reload() {
    std::string path = ResourceManager::BaseDir
        + "/Content/Dialogue/dialogue_" + LangSuffix(CurrentLanguage) + ".txt";
    std::ifstream f(path);
    if (!f.is_open()) {
        SDL_Log("DialogueManager::Reload: failed to open '%s'", path.c_str());
        // Fall back to English
        if (CurrentLanguage != Language::EN) {
            path = ResourceManager::BaseDir + "/Content/Dialogue/dialogue_EN.txt";
            f.open(path);
        }
        if (!f.is_open()) { SDL_Log("DialogueManager::Reload: fallback also failed"); return; }
    }
    SDL_Log("DialogueManager::Reload: opened '%s'", path.c_str());

    enum class ParseState { START, NPC, AREA, SCENE };
    ParseState pstate = ParseState::START;

    std::unordered_map<std::string, DialogueNPC> newTree;

    std::string npcName, areaName, sceneName;
    DialogueNPC* curNpc = nullptr;
    std::vector<std::string> curLines;
    int  loopID   = -1;
    bool alignTop = false;

    auto trim = [](std::string s) {
        size_t l = s.find_first_not_of(" \t\r\n");
        size_t r = s.find_last_not_of(" \t\r\n");
        if (l == std::string::npos) return std::string{};
        return s.substr(l, r - l + 1);
    };
    auto getToken = [](const std::string& line, int n) -> std::string {
        std::istringstream ss(line); std::string tok;
        for (int i = 0; i <= n; ++i) { if (!std::getline(ss, tok, ' ')) return ""; }
        return tok;
    };

    std::string line;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        switch (pstate) {
        case ParseState::START:
            if (line.rfind("npc ", 0) == 0) {
                npcName = getToken(line, 1);
                newTree[npcName] = DialogueNPC{};
                curNpc = &newTree[npcName];
                pstate = ParseState::NPC;
            }
            break;
        case ParseState::NPC:
            if (line == "does reset")  { curNpc->doesReset = true; }
            else if (line == "end npc") { pstate = ParseState::START; }
            else if (line.rfind("area ", 0) == 0) {
                areaName = getToken(line, 1);
                curNpc->AddArea(areaName);
                pstate = ParseState::AREA;
            }
            break;
        case ParseState::AREA:
            if (line == "end area") { pstate = ParseState::NPC; }
            else if (line.rfind("scene ", 0) == 0) {
                sceneName = getToken(line, 1);
                curLines.clear(); loopID = -1; alignTop = false;
                pstate = ParseState::SCENE;
            }
            break;
        case ParseState::SCENE:
            if (line == "end scene") {
                DialogueScene s;
                s.AlignTop = alignTop;
                s.LoopID   = loopID;
                s.lines    = curLines;
                // Preserve state from old SceneTree if it exists
                auto oit = SceneTree.find(npcName);
                if (oit != SceneTree.end()) {
                    auto* oa = oit->second.GetArea(areaName);
                    if (oa) {
                        auto* os = oa->GetScene(sceneName);
                        if (os) s.state = os->state;
                    }
                }
                curNpc->GetArea(areaName)->AddScene(sceneName, std::move(s));
                pstate = ParseState::AREA;
            }
            else if (line == "TOP")  { alignTop = true; }
            else if (line == "LOOP") { loopID = (int)curLines.size(); }
            else                     { curLines.push_back(line); }
            break;
        }
    }

    SceneTree = std::move(newTree);
}

void DialogueManager::SetLanguage(Language lang) {
    CurrentLanguage = lang;
    Reload();
}

DialogueScene* DialogueManager::GetScenePtr(const std::string& npc,
                                              const std::string& area,
                                              const std::string& scene) {
    auto ni = SceneTree.find(npc);
    if (ni == SceneTree.end()) return nullptr;
    auto* da = ni->second.GetArea(area);
    if (!da) return nullptr;
    return da->GetScene(scene);
}

std::string DialogueManager::GetDialogue(const std::string& npc, const std::string& area,
                                          const std::string& scene, int id) {
    auto* s = GetScenePtr(npc, area, scene);
    if (!s) 
    {
        SDL_Log("DialogueManager::GetDialogue: scene not found for npc='%s', area='%s', scene='%s'", npc.c_str(), area.c_str(), scene.c_str());
        return "No text available.";
    }
    
    Registry::GlobalState::DialogueTop = s->AlignTop;
    return ReplaceKeys(s->GetDialogue(id));
}

std::string DialogueManager::GetDialogue(const std::string& npc, const std::string& scene,
                                          int id) {
    return GetDialogue(npc, Registry::GlobalState::CURRENT_MAP_NAME, scene, id);
}

std::string DialogueManager::RandomDialogue(const std::string& npc, const std::string& scene) {
    return RandomDialogue(npc, Registry::GlobalState::CURRENT_MAP_NAME, scene);
}

std::string DialogueManager::RandomDialogue(const std::string& npc, const std::string& area,
                                              const std::string& scene) {
    auto* s = GetScenePtr(npc, area, scene);
    if (!s || s->Length() == 0) return "No text available.";
    return GetDialogue(npc, area, scene, Registry::GlobalState::RNG.Next(s->Length()));
}

bool DialogueManager::IsSceneDirty(const std::string& npc, const std::string& scene) {
    return IsSceneDirty(npc, Registry::GlobalState::CURRENT_MAP_NAME, scene);
}
bool DialogueManager::IsSceneFinished(const std::string& npc, const std::string& scene) {
    return IsSceneFinished(npc, Registry::GlobalState::CURRENT_MAP_NAME, scene);
}
bool DialogueManager::IsSceneDirty(const std::string& npc, const std::string& area, const std::string& scene) {
    auto* s = GetScenePtr(npc, area, scene); return s ? s->state.dirty : false;
}
bool DialogueManager::IsSceneFinished(const std::string& npc, const std::string& area, const std::string& scene) {
    auto* s = GetScenePtr(npc, area, scene); return s ? s->state.finished : false;
}
void DialogueManager::SetSceneProgress(const std::string& npc, const std::string& scene, int id) {
    auto* s = GetScenePtr(npc, Registry::GlobalState::CURRENT_MAP_NAME, scene);
    if (s) s->state.line = id;
}

static std::string GetKeyBoardString(Keys key) {
    switch (key) {
    case Keys::A: return "A"; case Keys::B: return "B"; case Keys::C: return "C";
    case Keys::D: return "D"; case Keys::E: return "E"; case Keys::F: return "F";
    case Keys::G: return "G"; case Keys::H: return "H"; case Keys::I: return "I";
    case Keys::J: return "J"; case Keys::K: return "K"; case Keys::L: return "L";
    case Keys::M: return "M"; case Keys::N: return "N"; case Keys::O: return "O";
    case Keys::P: return "P"; case Keys::Q: return "Q"; case Keys::R: return "R";
    case Keys::S: return "S"; case Keys::T: return "T"; case Keys::U: return "U";
    case Keys::V: return "V"; case Keys::W: return "W"; case Keys::X: return "X";
    case Keys::Y: return "Y"; case Keys::Z: return "Z";
    case Keys::D0: return "0"; case Keys::D1: return "1"; case Keys::D2: return "2";
    case Keys::D3: return "3"; case Keys::D4: return "4"; case Keys::D5: return "5";
    case Keys::D6: return "6"; case Keys::D7: return "7"; case Keys::D8: return "8";
    case Keys::D9: return "9";
    case Keys::Up: return "Up"; case Keys::Down: return "Down";
    case Keys::Left: return "Left"; case Keys::Right: return "Right";
    case Keys::Enter: return "Enter"; case Keys::Escape: return "Esc";
    case Keys::Space: return "Space"; case Keys::Back: return "Back";
    case Keys::Tab: return "Tab";
    case Keys::LeftShift: return "LShft"; case Keys::RightShift: return "RShft";
    case Keys::LeftControl: return "LC";  case Keys::RightControl: return "RC";
    case Keys::LeftAlt: return "LA";      case Keys::RightAlt: return "RA";
    case Keys::F1:  return "F1";  case Keys::F2:  return "F2";
    case Keys::F3:  return "F3";  case Keys::F4:  return "F4";
    case Keys::F5:  return "F5";  case Keys::F6:  return "F6";
    case Keys::F7:  return "F7";  case Keys::F8:  return "F8";
    case Keys::F9:  return "F9";  case Keys::F10: return "F10";
    case Keys::F11: return "F11"; case Keys::F12: return "F12";
    default: return "Key";
    }
}

static std::string GetButtonString(Buttons b) {
    // \xe2\x97\x86 = UTF-8 for U+25C6 BLACK DIAMOND
    static const char* diamond = "\xe2\x97\x86";
    int idx = -1;
    switch (b) {
    case Buttons::A:                    idx = 0;  break;
    case Buttons::B:                    idx = 1;  break;
    case Buttons::X:                    idx = 2;  break;
    case Buttons::Y:                    idx = 3;  break;
    case Buttons::Start:                idx = 4;  break;
    case Buttons::Back:                 idx = 5;  break;
    case Buttons::LeftShoulder:         idx = 6;  break;
    case Buttons::RightShoulder:        idx = 7;  break;
    case Buttons::LeftTrigger:          idx = 8;  break;
    case Buttons::RightTrigger:         idx = 9;  break;
    case Buttons::LeftStick:            idx = 10; break;
    case Buttons::RightStick:           idx = 11; break;
    case Buttons::LeftThumbstickUp:     idx = 13; break;
    case Buttons::LeftThumbstickDown:   idx = 14; break;
    case Buttons::LeftThumbstickLeft:   idx = 15; break;
    case Buttons::LeftThumbstickRight:  idx = 16; break;
    case Buttons::RightThumbstickUp:    idx = 17; break;
    case Buttons::RightThumbstickDown:  idx = 18; break;
    case Buttons::RightThumbstickLeft:  idx = 19; break;
    case Buttons::RightThumbstickRight: idx = 20; break;
    case Buttons::DPadUp:               idx = 21; break;
    case Buttons::DPadDown:             idx = 22; break;
    case Buttons::DPadLeft:             idx = 23; break;
    case Buttons::DPadRight:            idx = 24; break;
    default: idx = -1; break;
    }
    return std::string(diamond) + std::to_string(idx) + diamond;
}

std::string DialogueManager::ReplaceKeys(const std::string& line) {
    using KF = Input::KeyFunctions;
    using KI = Input::KeyInput;

    auto getKey = [](KF fn) -> Keys {
        auto it = KI::RebindableKeys.find(fn);
        if (it == KI::RebindableKeys.end() || it->second.mKeys.empty()) return Keys::None;
        return it->second.mKeys[0];
    };
    auto getBtn = [](KF fn) -> Buttons {
        auto it = KI::RebindableKeys.find(fn);
        if (it == KI::RebindableKeys.end() || it->second.ButtonsList.empty()) return Buttons::A;
        return it->second.ButtonsList[0];
    };
    auto rpl = [](std::string& s, const std::string& from, const std::string& to) {
        size_t pos;
        while ((pos = s.find(from)) != std::string::npos) s.replace(pos, from.size(), to);
    };

    std::string out = line;
    if (KI::ControllerMode) {
        rpl(out, "[SOMEKEY-X]",     GetButtonString(getBtn(KF::Cancel)));
        rpl(out, "[SOMEKEY-C]",     GetButtonString(getBtn(KF::Accept)));
        rpl(out, "[SOMEKEY-LEFT]",  GetButtonString(getBtn(KF::Left)));
        rpl(out, "[SOMEKEY-UP]",    GetButtonString(getBtn(KF::Up)));
        rpl(out, "[SOMEKEY-RIGHT]", GetButtonString(getBtn(KF::Right)));
        rpl(out, "[SOMEKEY-DOWN]",  GetButtonString(getBtn(KF::Down)));
        rpl(out, "[SOMEKEY-ENTER]", GetButtonString(getBtn(KF::Pause)));
    } else {
        rpl(out, "[SOMEKEY-X]",     GetKeyBoardString(getKey(KF::Cancel)));
        rpl(out, "[SOMEKEY-C]",     GetKeyBoardString(getKey(KF::Accept)));
        rpl(out, "[SOMEKEY-LEFT]",  GetKeyBoardString(getKey(KF::Left)));
        rpl(out, "[SOMEKEY-UP]",    GetKeyBoardString(getKey(KF::Up)));
        rpl(out, "[SOMEKEY-RIGHT]", GetKeyBoardString(getKey(KF::Right)));
        rpl(out, "[SOMEKEY-DOWN]",  GetKeyBoardString(getKey(KF::Down)));
        rpl(out, "[SOMEKEY-ENTER]", GetKeyBoardString(getKey(KF::Pause)));
    }
    return out;
}

} // namespace AnodyneSharp::Dialogue
