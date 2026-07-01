#pragma once
#include "AnodyneSharp/Dialogue/Tree/DialogueArea.hpp"

namespace AnodyneSharp::Dialogue::Tree {

// Represents an NPC's dialogue data — a named collection of scenes
class DialogueNPC {
public:
    explicit DialogueNPC(const std::string& name);

    void AddScene(const std::string& sceneName,
                  const std::string& speaker,
                  const std::vector<std::string>& lines);

    // Get a specific line for this NPC
    std::string GetLine(const std::string& scene, const std::string& speaker, int index) const;

    const std::string& Name() const { return _name; }

private:
    std::string _name;
    // scene -> speaker -> lines
    std::unordered_map<std::string,
        std::unordered_map<std::string,
        std::vector<std::string>>> _scenes;
};

} // namespace AnodyneSharp::Dialogue::Tree
