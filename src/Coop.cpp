#include "Coop.h"
#include "LevelScript.h"
#include "ScrollingTexture.h"
#include "MovingTexture.h"
#include "Sound.h"
#include "Tweak.h"

void ExportLua(N64Rom &Rom) {
    FILE *LuaDump = fopen("output/main.lua", "w");

    printf("Exporting lua file\n");

    fprintf(LuaDump, "-- name: %s\n\n", Rom.ROMInternalName.c_str());
    fprintf(LuaDump, "-- Manual changes are needed in this file for it to properly work.\n\n");

    if (!ScrollingTextures.empty()) {
        fprintf(LuaDump, "-- Scrolling Textures\n");

        fprintf(LuaDump,
            "local _add_scroll_target = add_scroll_target\n"
            "\n"
            "local function add_scroll_target(index, name, offset, numVerts)\n"
            "    local addr = tonumber(name:match(\"0x(%%x+)$\"), 16)\n"
            "    local prefix = name:match(\"(.-)0x\")\n"
            "    local baseAddr = addr\n"
            "\n"
            "    while not vtx_get_from_name(string.format(\"%%s0x%%x\", prefix, baseAddr)) do\n"
            "        baseAddr = baseAddr - 0x10\n"
            "    end\n"
            "\n"
            "    local i = 0\n"
            "\n"
            "    while i < numVerts do\n"
            "        local vAddr = addr + i * 0x10\n"
            "        local vbStart = baseAddr + ((vAddr - baseAddr) // 0xF0) * 0xF0\n"
            "        local vOffset = (vAddr - vbStart) // 0x10\n"
            "        local vCount = math.min(15 - vOffset, numVerts - i)\n"
            "\n"
            "        _add_scroll_target(index, string.format(\"%%s0x%%x\", prefix, vbStart), vOffset, vCount)\n"
            "\n"
            "        i = i + vCount\n"
            "    end\n"
            "end\n"
            "\n"
        );

        for (const auto &Scroll : ScrollingTextures) {
            fprintf(LuaDump, "--add_scroll_target(%u, \"%s_%u_vertex_0x%x\", %u, %u)\n", Scroll.Index, Scroll.LvlName.c_str(), Scroll.Area, Scroll.Addr, Scroll.Cycle, Scroll.NumVtx);
        }
    }

    for (const auto &MovTexQC : MovingTextures) {
        //force type for now cuz idk how ts works
        u16 Type = 1; /*MovTexQC.Type*/
        fprintf(LuaDump, "movtexqc_register(\"%s_%u_movtext_%u\", %u, %u, %u)\n", MovTexQC.LvlName.c_str(), MovTexQC.Area, MovTexQC.Index, MovTexQC.LvlID, MovTexQC.Area, Type);
    }
    if (SoundExport) {
        fprintf(LuaDump, "\n-- Audio\n");
        bool UseNames = !SequenceNames.empty();
        for (const auto &Music : SequenceMusics) {
            if (UseNames && Music < SequenceNames.size()) {
                std::string Name = SequenceNames[Music];
                fprintf(LuaDump, "smlua_audio_utils_replace_sequence(0x%02x, 0x%02x, %u, \"%s\")\n", Music, GetSeqNInst(Rom, Music), 80, Name.c_str());
            } else {
                fprintf(LuaDump, "smlua_audio_utils_replace_sequence(0x%02x, 0x%02x, %u, \"seq_0x%02x\")\n", Music, GetSeqNInst(Rom, Music), 80, Music);
            }
        }
    }
    if (TweakExport) {
        if (GameType == GT_ROM_MANAGER || GameType == GT_EDITOR) {
            fprintf(LuaDump, "\n-- Tweaks\n%s", GetRomTweaks(Rom).c_str());
        }
    }
    fclose(LuaDump);
}