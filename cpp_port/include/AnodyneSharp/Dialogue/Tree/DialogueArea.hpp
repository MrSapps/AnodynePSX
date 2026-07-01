#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp::Dialogue::Tree {

// A single line or choice within a dialogue tree node
struct DialogueLine {
    std::string speaker;
    std::string text;
    int         nextNode = -1;  // -1 = end
};

// A dialogue node containing one or more lines
struct DialogueNode {
    int                        id;
    std::vector<DialogueLine>  lines;
    bool                       isChoice = false;
};

// A named dialogue area (file-level grouping of scenes and NPCs)
struct DialogueArea {
    std::string name;
    // map: scene name -> map: speaker -> sequence of lines
    std::unordered_map<std::string,
        std::unordered_map<std::string,
        std::vector<std::string>>> scenes;
};

} // namespace AnodyneSharp::Dialogue::Tree
