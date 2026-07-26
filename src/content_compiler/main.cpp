#include <stdio.h>
#include <string>
#include <vector>
#include <optional>
#include <map>
#include <fstream>

namespace string
{
    std::vector<std::string> split(const std::string &str, const std::string &delimiter)
    {
        std::vector<std::string> result;

        if (delimiter.empty())
        {
            result.push_back(str);
            return result;
        }

        std::size_t start = 0;
        while (true)
        {
            std::size_t pos = str.find(delimiter, start);
            if (pos == std::string::npos)
            {
                result.push_back(str.substr(start));
                break;
            }

            result.push_back(str.substr(start, pos - start));
            start = pos + delimiter.size();
        }

        return result;
    }

    bool startsWith(const std::string &str, const std::string &startsWithStr)
    {
        if (startsWithStr.size() > str.size())
            return false;

        return std::equal(
            startsWithStr.begin(),
            startsWithStr.end(),
            str.begin());
    }

    bool IsEmptyOrWhiteSpace(const std::string &str)
    {
        if (str.empty())
        {
            return true;
        }

        for (std::string::size_type i = 0; i < str.size(); ++i)
        {
            if (!std::isspace(static_cast<unsigned char>(str[i])))
            {
                return false;
            }
        }
        return true;
    }

    std::string trim(const std::string &s)
    {
        std::string::size_type start = 0;
        std::string::size_type end = s.size();

        // Find first non‑whitespace
        while (start < end && std::isspace(static_cast<unsigned char>(s[start])))
        {
            ++start;
        }

        // Find last non‑whitespace
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        {
            --end;
        }

        return s.substr(start, end - start);
    }
}

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
        //[JsonInclude]
        bool dirty = false;
        // [JsonInclude]
        bool finished = false;
        // [JsonInclude]
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

        std::ifstream textStream(fullPath.c_str());
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

int main(int argc, char **argv)
{
    printf("Content compiler exec from %s\n", argv[0]);

    DialogCompiler dc;
    if (dc.Compile("../../old/Content/Dialogue", "EN"))
    {
        printf("Dialog compiled %d scenes\n", dc.newSceneTree.size());
        for (auto& [sceneName, sceneDialogNpc] : dc.newSceneTree)
        {
            for (auto& [npcName, area] : sceneDialogNpc._areas)
            {
                for (auto& [areaName, sceneDialog] : area._scenes)
                {
                    printf("scene name [%s] npc name [%s] area name [%s] lines [%d]\n", sceneName.c_str(), npcName.c_str(), areaName.c_str(), sceneDialog._lines.size());
                    if (!sceneDialog._lines.empty())
                    {
                        printf(" lines:\n");
                        for (auto& line : sceneDialog._lines)
                        {
                            printf("  %s\n", line.c_str());
                        }
                    }
                }
            }
        }
    }
    else
    {
        printf("Dialog OVER!\n");
    }

    return 0;
}
