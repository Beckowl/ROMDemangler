#include "Collision.h"

void ExportCollision(N64Rom &Rom, u8 Area, const std::string &LvlName, u32 SegAddr, u32 &Entry, LevelScript &Script, const char *FilePath) {
    Entry = (SegAddr);
    FILE *ColDump = fopen(FilePath, "w");
    fprintf(ColDump, "const Collision %s_area_%u_collision[] = {\n", LvlName.c_str(), Area);
    fprintf(ColDump, "    COL_INIT(),\n");

    s16 NumVerts = Rom.ReadBytes<s16>(Entry + 2);
    fprintf(ColDump, "    COL_VERTEX_INIT(%d),\n", NumVerts);
    Entry += 4;

    // read verts
    for (s32 V = 0; V < NumVerts; V++) {
        s16 X = Rom.ReadBytes<s16>(Entry + V * 6 + 0);
        s16 Y = Rom.ReadBytes<s16>(Entry + V * 6 + 2);
        s16 Z = Rom.ReadBytes<s16>(Entry + V * 6 + 4);
        fprintf(ColDump, "    COL_VERTEX(%d, %d, %d),\n", X, Y, Z);
    }

    Entry += NumVerts * 6;
    s32 XOffset = 0;
    s32 Guard = 0;
    const std::set<s16> SpecialTris = {0x0E, 0x24, 0x25, 0x27, 0x2C, 0x2D};

    // read tris
    while (true) {
        s16 SurfType = Rom.ReadBytes<s16>(Entry + XOffset + 0);
        s16 NumTris = Rom.ReadBytes<s16>(Entry + XOffset + 2);
        if (SurfType == 0x41 || Guard > 20000) {
            fprintf(ColDump, "    COL_TRI_STOP(),\n");
            if (Guard > 50000) {
                printf("Level has broken collision data, stopping collision export");
            }
            break;
        }
        fprintf(ColDump, "    COL_TRI_INIT(%d, %d),\n", SurfType, NumTris);

        if (SpecialTris.count(SurfType)) {
            for (s32 T = 0; T < NumTris; T++) {
                s16 V1 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 8 + 0);
                s16 V2 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 8 + 2);
                s16 V3 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 8 + 4);
                s16 Param = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 8 + 6);
                fprintf(ColDump, "    COL_TRI_SPECIAL(%d, %d, %d, %d),\n", V1, V2, V3, Param);
            }
            XOffset += NumTris * 8 + 4;
        } else {
            for (s32 T = 0; T < NumTris; T++) {
                s16 V1 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 6 + 0);
                s16 V2 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 6 + 2);
                s16 V3 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 6 + 4);
                fprintf(ColDump, "    COL_TRI(%d, %d, %d),\n", V1, V2, V3);
            }
            XOffset += NumTris * 6 + 4;
        }
        Guard++;
    }

    // read special
    Guard = 0;
    XOffset += 2;
    while (true) {
        s16 SpecialType = Rom.ReadBytes<s16>(Entry + XOffset + 0);
        s16 Stuff = Rom.ReadBytes<s16>(Entry + XOffset + 2);
        if (SpecialType == 0x42 || Guard > 20000) {
            if (Guard > 50000) {
                printf("Level has broken collision data, stopping collision export");
            }
            fprintf(ColDump, "    COL_END(),\n");
            break;
        } else if (SpecialType == 0x44) {
            fprintf(ColDump, "    COL_WATER_BOX_INIT(%d),\n", Stuff);
            Script.AreaDatas[Area].WaterBoxCount = Stuff;
            XOffset += 4;
            for (s32 W = 0; W < Stuff; W++) {
                //id, x1, z1, x2, z2, y
                s16 ID = Rom.ReadBytes<s16>(Entry + XOffset + 0);
                s16 X1 = Rom.ReadBytes<s16>(Entry + XOffset + 2);
                s16 Z1 = Rom.ReadBytes<s16>(Entry + XOffset + 4);
                s16 X2 = Rom.ReadBytes<s16>(Entry + XOffset + 6);
                s16 Z2 = Rom.ReadBytes<s16>(Entry + XOffset + 8);
                s16 Y = Rom.ReadBytes<s16>(Entry + XOffset + 10);
                fprintf(ColDump, "    COL_WATER_BOX(%d, %d, %d, %d, %d, %d),\n", ID, X1, Z1, X2, Z2, Y);
                XOffset += 12;
            }
        } else {
            fprintf(ColDump, "    COL_END(),\n");
            break;
        }
        Guard++;
    }

    fprintf(ColDump, "};\n");
    fclose(ColDump);
}