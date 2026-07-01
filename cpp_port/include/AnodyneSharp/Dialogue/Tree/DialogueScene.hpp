#pragma once
#include "AnodyneSharp/Dialogue/Tree/DialogueNPC.hpp"

namespace AnodyneSharp::Dialogue::Tree {

// Tracks the progression through a dialogue sequence
class DialogueScene {
public:
    DialogueScene() = default;
    explicit DialogueScene(const DialogueNPC* npc,
                           const std::string& sceneName,
                           const std::string& speaker);

    bool        HasNext() const;
    std::string Next();
    void        Reset();

    bool IsEmpty() const { return _lines.empty(); }

private:
    std::vector<std::string> _lines;
    int                      _index = 0;
};

} // namespace AnodyneSharp::Dialogue::Tree
