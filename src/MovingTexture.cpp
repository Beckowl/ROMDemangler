#include "MovingTexture.h"
#include "LevelScript.h"

std::vector<MovingTextureQC> MovingTextures = {};

// this is the most depressing function i have ever wrote
void ExportMovText(N64Rom &Rom, u8 Area, std::string LvlName, LevelScript &Script, const char *FilePath) {
    FILE *MovTextDump = fopen(FilePath, "w");
    u8 WaterCount = Script.AreaDatas[Area].WaterBoxCount;
    if (WaterCount == 0) {
        fclose(MovTextDump);
        return;
    }
    for (u32 DT = 0; DT < Script.AreaDatas[Area].WaterBoxParams.size(); DT++) {
        s16 WaterType = Script.AreaDatas[Area].WaterBoxParams.data()[DT] & 0xff;
        u32 WaterBox = 0;
        std::vector<u32> MovTextPtrs = {};
        std::vector<std::string> MovTexQCStrings = {};

        if (GameType == GT_EDITOR) {
            WaterBox = (0x19001800+0x50*WaterType);
        } else {
            WaterBox = (0x19006000+0x280*WaterType+0x50*Area);
        }
        
        for (u32 W = 0; W < WaterCount; W++) {
            u32 Ptr = Rom.ReadBytes<u32>(WaterBox + 4 + W * 8);
            if (Ptr == 0) break;
            MovTextPtrs.push_back((Ptr));
        }

        u32 MovTextID = 0;
        for (u32 MT = 0; MT < MovTextPtrs.size(); MT++) {
            u32 Entry = MovTextPtrs.data()[MT];
            char MovTexStr[1024];
            snprintf(MovTexStr, 1024, "%s_%u_movtext_%u_%u", LvlName.c_str(), Area, DT, MovTextID);
            fprintf(MovTextDump, "static Movtex %s[] = {", MovTexStr);
            MovTexQCStrings.push_back(MovTexStr);

            for (u32 WD = 0; WD < 0x20; WD+=2) {
                fprintf(MovTextDump, "%d",
                Rom.ReadBytes<s16>(Entry + WD));
                if (WD < 30) fprintf(MovTextDump, ", ");
            }
            fprintf(MovTextDump, "};\n");
            MovTextID++;
        }
        fprintf(MovTextDump, "const struct MovtexQuadCollection %s_%u_movtext_%u[] = {\n", LvlName.c_str(), Area, DT);
        for (u32 MSTR = 0; MSTR < MovTexQCStrings.size(); MSTR++) {
            fprintf(MovTextDump, "    {%u, %s},\n", MSTR, MovTexQCStrings.data()[MSTR].c_str());
        }
        fprintf(MovTextDump, "    {-1, NULL},\n");
        fprintf(MovTextDump, "};\n");

        MovingTextureQC NewMovTexQC;
        NewMovTexQC.LvlName = LvlName;
        NewMovTexQC.LvlID = Script.LevelID;
        NewMovTexQC.Area = Area;
        NewMovTexQC.Type = WaterType;
        NewMovTexQC.Index = DT;
        MovingTextures.push_back(NewMovTexQC);
    }
    fclose(MovTextDump);
}