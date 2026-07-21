#include "Coop.h"
#include "LevelScript.h"
#include "ScrollingTexture.h"

void ExportLua(N64Rom &Rom) {
    FILE *LuaDump = fopen("output/main.lua", "w");
    fprintf(LuaDump, "-- name: %s\n\n", Rom.mRomInternalName.c_str());
    fprintf(LuaDump, "-- Manual changes are needed in this file for it to properly work.\n\n");
    for (const auto &Scroll : ScrollingTextures) {
        fprintf(LuaDump, "--add_scroll_target(%u, \"%s_%u_vertex_0x%x\"--[[, %u, %u]])\n", Scroll.Index, Scroll.LvlName.c_str(), Scroll.Area, Scroll.Addr, Scroll.Cycle, Scroll.NumVtx);
    }
    fclose(LuaDump);
}