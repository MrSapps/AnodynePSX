#include "AnodyneSharp/Dialogue/Tree/DialogueNPC.hpp"
#include "AnodyneSharp/Dialogue/Tree/DialogueScene.hpp"

namespace AnodyneSharp::Dialogue::Tree {

// DialogueNPC
DialogueNPC::DialogueNPC(const std::string& name) : _name(name) {}

void DialogueNPC::AddScene(const std::string& sceneName,
                            const std::string& speaker,
                            const std::vector<std::string>& lines) {
    _scenes[sceneName][speaker] = lines;
}

std::string DialogueNPC::GetLine(const std::string& scene,
                                  const std::string& speaker, int index) const {
    auto sit = _scenes.find(scene);
    if (sit == _scenes.end()) return "";
    auto spk = sit->second.find(speaker);
    if (spk == sit->second.end()) return "";
    if (index < 0 || index >= (int)spk->second.size()) return "";
    return spk->second[index];
}

// DialogueScene
DialogueScene::DialogueScene(const DialogueNPC* npc,
                              const std::string& sceneName,
                              const std::string& speaker) {
    if (!npc) return;
    // Collect all lines from this NPC for this scene + speaker
    for (int i = 0; ; ++i) {
        auto l = npc->GetLine(sceneName, speaker, i);
        if (l.empty()) break;
        _lines.push_back(l);
    }
}

bool DialogueScene::HasNext() const { return _index < (int)_lines.size(); }

std::string DialogueScene::Next() {
    if (!HasNext()) return "";
    return _lines[_index++];
}

void DialogueScene::Reset() { _index = 0; }

} // namespace AnodyneSharp::Dialogue::Tree
