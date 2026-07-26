#include <stdio.h>
#include "dialog_compiler.hpp"

static void dump_dialog(DialogCompiler &dc)
{
    printf("Dialog compiled %d scenes\n", dc.newSceneTree.size());
    for (auto &[npcName, sceneDialogNpc] : dc.newSceneTree)
    {
        for (auto &[areaName, area] : sceneDialogNpc._areas)
        {
            for (auto &[sceneName, sceneDialog] : area._scenes)
            {
                printf("scene name [%s] npc name [%s] area name [%s] lines [%d]\n", sceneName.c_str(), npcName.c_str(), areaName.c_str(), sceneDialog._lines.size());
                if (!sceneDialog._lines.empty())
                {
                    printf(" lines:\n");
                    for (auto &line : sceneDialog._lines)
                    {
                        printf("  %s\n", line.c_str());
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    printf("Content compiler exec from %s\n", argv[0]);

    DialogCompiler dc;
    if (dc.Compile("../../old/Content/Dialogue", "EN"))
    {
        dump_dialog(dc);
    }
    else
    {
        printf("Dialog OVER!\n");
    }

    return 0;
}
