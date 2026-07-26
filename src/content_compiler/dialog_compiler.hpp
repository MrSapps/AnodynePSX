#pragma once

#include "string.hpp"
#include <optional>
#include <map>

// Compile Content/Dialogue/dialogue_[LANG_CODE].txt into a binary file
// that is used by the engine
class DialogCompiler final
{
public:
    enum class ParseState
    {
        START,
        NPC,
        AREA,
        SCENE,
    };

    class DialogueState final
    {
    public:
        bool dirty = false;
        bool finished = false;
        int line = 0;
    };

    class DialogueScene final
    {
    public:
        DialogueScene() {}

        DialogueScene(bool alignTop, std::optional<std::size_t> loopId, std::vector<std::string> &dialog)
            : AlignTop(alignTop), LoopID(loopId), _lines(dialog)
        {
        }

        bool AlignTop = false;
        std::optional<std::size_t> LoopID;
        std::vector<std::string> _lines;

        DialogueState state;
    };

    class DialogueArea final
    {
    public:
        std::map<std::string, DialogueScene> _scenes;
    };

    class DialogueNPC final
    {
    public:
        bool doesReset;

        std::map<std::string, DialogueArea> _areas;
    };

    std::string GetName(std::string line)
    {
        return string::split(line, " ")[1];
    }

    std::map<std::string, DialogueNPC> newSceneTree;

    bool Compile(const std::string &path, const std::string &langCode)
    {
        const std::string fullPath = path + "/dialogue_" + langCode + ".txt";
        printf("Path = %s\n", fullPath.c_str());

        std::ifstream textStream(fullPath.c_str(), std::ios::in);
        if (!textStream.is_open())
        {
            return false;
        }

        ParseState state = ParseState::START;

        DialogueNPC* npc = nullptr;
        std::string npcName;
        std::string areaName;
        std::string scene;

        std::vector<std::string> dialogue;
        std::optional<std::size_t> loopID;
        bool alignTop = false;

        std::string line;
        while (std::getline(textStream, line))
        {
            line = string::trim(line);
            // printf("line = %s\n", line.c_str());

            // Skip comments
            if (string::startsWith(line, "#"))
            {
                continue;
            }

            switch (state)
            {
            case ParseState::START:
                if (string::startsWith(line, "npc"))
                {
                    state = ParseState::NPC;
                    npcName = GetName(line);
                    newSceneTree[npcName] = {};
                    npc = &newSceneTree[npcName];
                }
                break;

            case ParseState::NPC:
                if (line == "does reset")
                {
                    npc->doesReset = true;
                    continue;
                }
                if (line == "end npc")
                {
                    state = ParseState::START;
                    continue;
                }
                if (string::startsWith(line, "area"))
                {
                    areaName = GetName(line);
                    state = ParseState::AREA;
                    npc->_areas[areaName] = {};
                }
                break;

            case ParseState::AREA:
                if (line == "end area")
                {
                    state = ParseState::NPC;
                    continue;
                }

                if (string::startsWith(line, "scene"))
                {
                    scene = GetName(line);
                    state = ParseState::SCENE;

                    loopID = {};
                    dialogue.clear();
                    alignTop = false;
                }
                break;

            case ParseState::SCENE:
                if (string::IsEmptyOrWhiteSpace(line))
                {
                    continue;
                }

                if (line == "TOP")
                {
                    alignTop = true;
                    continue;
                }
                if (line == "end scene")
                {
                    state = ParseState::AREA;
                    DialogueScene s(alignTop, loopID, dialogue);
                    // TODO: Not required ??
                    // s.state = newSceneTree[npcName].GetArea(areaName)->_scenes[scene].state;
                    npc->_areas[areaName]._scenes[scene] = s;
                    continue;
                }
                if (line == "LOOP")
                {
                    loopID = dialogue.size();
                    continue;
                }
                dialogue.push_back(line);
                break;
            }
        }
        return true;
    }
};
