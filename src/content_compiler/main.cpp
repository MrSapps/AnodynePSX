#include <stdio.h>
#include <string>
#include <vector>
#include <optional>
#include <map>
#include <fstream>

namespace string
{
    bool startsWith(const std::string& str, const std::string& startsWithStr)
    {
        if (startsWithStr.size() > str.size())
            return false;

        return std::equal(
            startsWithStr.begin(),
            startsWithStr.end(),
            str.begin()
        );
    }

    bool IsEmptyOrWhiteSpace(const std::string& str)
    {
        if (str.empty())
        {
            return true;
        }

        for (std::string::size_type i = 0; i < str.size(); ++i)
        {
            if (!std::isspace(static_cast<unsigned char>(str[i])))
                return false;
        }
        return true;
    }

    std::string trim(const std::string& s)
    {
        std::string::size_type start = 0;
        std::string::size_type end = s.size();

        // Find first non‑whitespace
        while (start < end && std::isspace(static_cast<unsigned char>(s[start])))
            ++start;

        // Find last non‑whitespace
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
            --end;

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
        DialogueScene(bool alignTop, std::optional<std::size_t> loopId, std::vector<std::string>& dialog)
        {

        }

        /*
        [JsonIgnore]
        public bool AlignTop { get; private set; }
        [JsonIgnore]
        public int? LoopID { get; private set; }

        [JsonInclude]
        public DialogueState state;

        private List<string> _lines;

        [JsonIgnore]
        public int Length => _lines.Count;
        */
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

    /*
    std::string GetDialogue(const std::string& npc, const std::string& area, const std::string& scene, int id = -1)
        {
            DialogueScene* s = GetScene(npc, area, scene);
            if(s == nullptr)
            {
                return "No text available.";
            }

            //GlobalState.DialogueTop = s.AlignTop;

            //return ReplaceKeys(s.GetDialogue(id));
        }*/

        std::string GetName(std::string line)
        {
//            return line.Split(' ')[1];
// TODO
return "";
        }

    bool Compile(const std::string &path, const std::string& langCode)
    {
        const std::string fullPath = path + "/dialogue_" + langCode + ".txt";
        printf("Path = %s\n", fullPath.c_str());

        std::ifstream textStream(fullPath.c_str());
        if (!textStream.is_open())
        {
            return false;
        }


        ParseState state = ParseState::START;

        std::map<std::string, DialogueNPC> newSceneTree;

        DialogueNPC npc;
        std::string npcName;
        std::string areaName;
        std::string scene;

        std::vector<std::string> dialogue;
        std::optional<std::size_t> loopID;
        bool alignTop = false;

        //using Stream stream = AssemblyReaderUtil.GetStream(path);
        //using StreamReader reader = new StreamReader(stream);

        std::string line;
        while(std::getline(textStream, line))
        {
            line = string::trim(line);

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
                    //npc = new DialogueNPC();
                    state = ParseState::NPC;
                    npcName = GetName(line);
                    newSceneTree[npcName] = npc;
                }
                break;

            case ParseState::NPC:
                if (line == "does reset")
                {
                    npc.doesReset = true;
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
                    //npc.AddArea(areaName);
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
                    //npc.GetArea(areaName).AddScene(scene, s);
                    //s.state = GetScene(npcName, areaName, scene).state;
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
        //SceneTree = newSceneTree;

        return true;
    }
};

int main(int argc, char **argv)
{
    printf("Content compiler exec from %s\n", argv[0]);

    DialogCompiler dc;
    if (dc.Compile("../../old/Content/Dialogue", "EN"))
    {
        printf("Dialog compiled\n");
    }
    else
    {
        printf("Dialog OVER!\n");
    }

    return 0;
}
