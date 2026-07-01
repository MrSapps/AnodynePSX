#pragma once
#include "AnodyneSharp/Common.hpp"
#include "AnodyneSharp/Registry/Settings.hpp"  // defines Language enum

namespace AnodyneSharp::Dialogue {

// ---- Scene state (persisted across saves) ----------------------------------
struct DialogueState {
    int  line     = 0;
    bool dirty    = false;
    bool finished = false;
};

// ---- DialogueScene ---------------------------------------------------------
struct DialogueScene {
    bool      AlignTop = false;
    int       LoopID   = -1;   // -1 = no loop
    DialogueState state;
    std::vector<std::string> lines;

    int Length() const { return (int)lines.size(); }

    std::string GetDialogue(int id = -1) {
        state.dirty = true;
        if (id == -1) id = state.line;
        if (id >= (int)lines.size()) id = (LoopID >= 0 ? LoopID : 0);
        if (lines.empty()) return "";
        std::string line = lines[id];
        id++;
        if (id >= (int)lines.size()) state.finished = true;
        state.line = id;
        return line;
    }
};

// ---- DialogueArea ----------------------------------------------------------
struct DialogueArea {
    std::unordered_map<std::string, DialogueScene> scenes;

    DialogueScene* GetScene(const std::string& name) {
        auto it = scenes.find(name);
        return it != scenes.end() ? &it->second : nullptr;
    }
    void AddScene(const std::string& name, DialogueScene s) { scenes[name] = std::move(s); }
};

// ---- DialogueNPC -----------------------------------------------------------
struct DialogueNPC {
    bool doesReset = false;
    std::unordered_map<std::string, DialogueArea> areas;

    DialogueArea* GetArea(const std::string& name) {
        auto it = areas.find(name);
        return it != areas.end() ? &it->second : nullptr;
    }
    void AddArea(const std::string& name) { areas[name] = DialogueArea{}; }
};

// ---- DialogueManager -------------------------------------------------------
class DialogueManager {
public:
    static Language CurrentLanguage;

    // SceneTree: npc → DialogueNPC (saved/loaded with game)
    static std::unordered_map<std::string, DialogueNPC> SceneTree;

    static void Reload();
    static void SetLanguage(Language lang);

    // Get a specific line by index (or -1 to use scene progress)
    static std::string GetDialogue(const std::string& npc, const std::string& area,
                                   const std::string& scene, int id = -1);
    static std::string GetDialogue(const std::string& npc, const std::string& scene,
                                   int id = -1);  // uses CURRENT_MAP_NAME as area

    static std::string RandomDialogue(const std::string& npc, const std::string& scene);
    static std::string RandomDialogue(const std::string& npc, const std::string& area,
                                      const std::string& scene);

    static bool IsSceneDirty   (const std::string& npc, const std::string& scene);
    static bool IsSceneFinished(const std::string& npc, const std::string& scene);
    static bool IsSceneDirty   (const std::string& npc, const std::string& area, const std::string& scene);
    static bool IsSceneFinished(const std::string& npc, const std::string& area, const std::string& scene);
    static void SetSceneProgress(const std::string& npc, const std::string& scene, int id);

    static std::string ReplaceKeys(const std::string& line);

private:
    static DialogueScene* GetScenePtr(const std::string& npc, const std::string& area,
                                       const std::string& scene);
};

} // namespace AnodyneSharp::Dialogue

using AnodyneSharp::Dialogue::DialogueManager;
using AnodyneSharp::Dialogue::DialogueNPC;
using AnodyneSharp::Dialogue::DialogueScene;
using AnodyneSharp::Dialogue::DialogueArea;
