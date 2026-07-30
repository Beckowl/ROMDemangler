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
    fprintf(LuaDump, "-- Scrolling Textures\n");
    for (const auto &Scroll : ScrollingTextures) {
        fprintf(LuaDump, "--add_scroll_target(%u, \"%s_%u_vertex_0x%x\"--[[, %u, %u]])\n", Scroll.Index, Scroll.LvlName.c_str(), Scroll.Area, Scroll.Addr, Scroll.Cycle, Scroll.NumVtx);
    }
    for (const auto &MovTexQC : MovingTextures) {
        //force type for now cuz idk how ts works
        u16 Type = 1; /*MovTexQC.Type*/
        fprintf(LuaDump, "movtexqc_register(\"%s_%u_movtext_%u\", %u, %u, %u)\n", MovTexQC.LvlName.c_str(), MovTexQC.Area, MovTexQC.Index, MovTexQC.LvlID, MovTexQC.Area, Type);
    }
    fprintf(LuaDump, "\n-- Audio\n");
    for (const auto &Music : SequenceMusics) {
        fprintf(LuaDump, "smlua_audio_utils_replace_sequence(0x%02x, 0x%02x, %u, \"seq_0x%02x\")\n", Music, GetSeqNLST(Rom, Music), 80, Music);
    }
    if (GameType == GT_ROM_MANAGER || GameType == GT_EDITOR) {
        fprintf(LuaDump, "\n-- Tweaks\n%s", GetRomTweaks(Rom).c_str());
    }
    fclose(LuaDump);
}