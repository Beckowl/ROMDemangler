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

        for (const auto &Scroll : ScrollingTextures) {
            fprintf(LuaDump, "add_scroll_target(0x%08x, \"%s\", %u, %u)\n", Scroll.Id, Scroll.Name.c_str(), Scroll.Offset, Scroll.NumVtx);
        }
    }

    for (const auto &MovTexQC : MovingTextures) {
        //force type for now cuz idk how ts works
        u16 Type = 0; /*MovTexQC.Type*/
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
        if (GameType.IsOldBinary()) {
            fprintf(LuaDump, "\n-- Tweaks\n%s", GetRomTweaks(Rom).c_str());
        }
    }
    fclose(LuaDump);
}