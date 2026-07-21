#include "Model.h"
#include "LevelScript.h"
#include "Actor.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const char *F3D_CC(enum F3DCCPart Part, u16 Element) {
    if (Part == CC_PART_A) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "1";
            case 0x7: return "NOISE";
            default:  return "0";
        }
    } else if (Part == CC_PART_B) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "CENTER";
            case 0x7: return "K4";
            default:  return "0";
        }
    } else if (Part == CC_PART_C) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "SCALE";
            case 0x7: return "COMBINED_ALPHA";
            case 0x8: return "TEXEL0_ALPHA";
            case 0x9: return "TEXEL1_ALPHA";
            case 0xA: return "PRIMITIVE_ALPHA";
            case 0xB: return "SHADE_ALPHA";
            case 0xC: return "ENV_ALPHA";
            case 0xD: return "LOD_FRACTION";
            case 0xE: return "PRIM_LOD_FRAC";
            case 0xF: return "K5";
            default:  return "0";
        }
    } else if (Part == CC_PART_D) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "1";
            default:  return "0";
        }
    }

    return "0";
}

const char *F3D_AC(enum F3DCCPart Part, u16 Element) {
    if (Part == CC_PART_A) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "1";
            default:  return "0";
        }
    } else if (Part == CC_PART_B) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "1";
            default:  return "0";
        }
    } else if (Part == CC_PART_C) {
        switch (Element) {
            case 0x0: return "LOD_FRACTION";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "PRIM_LOD_FRAC";
            default:  return "0";
        }
    } else if (Part == CC_PART_D) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "1";
            default:  return "0";
        }
    }

    return "0";
}


F3DTexture &EnsureActiveTexture(std::vector<F3DTexture> &Textures) {
    if (Textures.empty()) {
        F3DTexture t = {};
        t.TextureSeg = 0;
        t.Texture = 0;
        t.Tile = 0xFF; // fucked
        t.ImgType = F3D_IMG_RGBA;
        t.BitDepth = G_IM_SIZ_16b;
        t.Length = 0;
        t.Width = 0;
        t.Height = 0;
        t.Palette = 0;
        t.PaletteSeg = 0;
        Textures.push_back(t);
    }
    return Textures.back();
}

void ParseRDPTileCommand(std::vector<F3DTexture> &Textures, u32 W0, u32 W1, u8 Cmd, bool Write, bool IsActor = false, Actor *Act = nullptr, FILE *ModelDump = nullptr, std::string LvlName="", u8 Area=0) {
    if (!Write) {
        auto GetBitDepthFromSize = [](u32 size) {
            switch (size) {
                case G_IM_SIZ_4b:  return 4;
                case G_IM_SIZ_8b:  return 8;
                case G_IM_SIZ_16b: return 16;
                case G_IM_SIZ_32b: return 32;
                default:           return 16;
            }
        };

        switch (Cmd) {
            case G_SETTIMG: {
                // extremely fucked up hack because of old level editors
                // they spammed 2 set images back to back which,
                // ofc the last one will override the old one
                // but i still gotta export it otherwise
                // its gonna say im missing a texture

                // but... this doesnt work for some hacks and ends up
                // exporting some wrong textures :(
                if (!Textures.empty() && C0(21, 3) != F3D_IMG_CI) {
                    F3DTexture &Current = Textures.back();
                    if (Current.Texture != 0 && Current.Texture != W1) {
                        F3DTexture NewTex = Current; 
                        NewTex.TextureSeg = W1;
                        NewTex.Texture = W1;
                        NewTex.Width = 0;
                        NewTex.Height = 0;
                        NewTex.Length = 0;
                        
                        Textures.push_back(NewTex);
                        break; 
                    }
                }

                F3DTexture &Tex = EnsureActiveTexture(Textures);
                Tex.TextureSeg = W1;
                Tex.Texture = W1;
                Tex.ImgType = (F3DImageType)C0(21, 3);
                Tex.BitDepth = GetBitDepthFromSize(C0(19, 2));
                break;
            }
            
            case G_SETTILE: {
                F3DTexture &Tex = EnsureActiveTexture(Textures);
                
                u32 Tile = C1(24, 3);
                u32 FMT = C0(21, 3);
                u32 Siz = C0(19, 2);

                
                Tex.Tile = Tile;
                Tex.ImgType = (F3DImageType)FMT;
                Tex.BitDepth = GetBitDepthFromSize(Siz);

                if (Tex.Tile == 0xFF || Tex.Tile == Tile) {
                    Tex.Tile = Tile;
                    if (FMT != G_IM_FMT_CI) {
                        Tex.ImgType = (F3DImageType)FMT;
                        Tex.BitDepth = GetBitDepthFromSize(Siz);
                    } else {
                    }
                }
                break;
            }
            
            case G_LOADBLOCK: {
                F3DTexture &Tex = EnsureActiveTexture(Textures);
                u32 Texels = C1(12, 12);
                Tex.Length = ((Texels + 1) * Tex.BitDepth) / 8; 
                break;
            }
            
            case G_SETTILESIZE: {
                F3DTexture &Tex = EnsureActiveTexture(Textures);

                u16 Uls = C0(12,12), Ult = C0(0,12);
                u16 Lrs = C1(12,12), Lrt = C1(0,12);

                Tex.Width = (u16)(((Lrs >> 2) - (Uls >> 2)) + 1);
                Tex.Height = (u16)(((Lrt >> 2) - (Ult >> 2)) + 1);
                
                break;
            }
            
            case G_LOADTLUT: {
                F3DTexture &Tex = EnsureActiveTexture(Textures);
                
                if (C1(24,3) == Tex.Tile) {
                    Tex.Palette = Tex.Texture;
                    Tex.PaletteSeg = Tex.TextureSeg;
                }
                
                break;
            }
        }
    } else {
        auto GetPlaceHolderName = [LvlName, Area, IsActor, Act](void) {
            return IsActor ? Act->Name : std::format("{}_{}", LvlName, Area);
        };
        switch (Cmd) {
            case G_SETTIMG: fprintf(ModelDump,"    gsDPSetTextureImage(%u, %u, %u, %s_texture_0x%x),\n",
                            C0(21,3),C0(19,2),1,GetPlaceHolderName().c_str(),W1); break;
            case G_SETTILE: fprintf(ModelDump,"    gsDPSetTile(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u),\n",
                            C0(21,3),C0(19,2),C0(9,9),C0(0,9),C1(24,3),C1(20,4),C1(18,2),C1(14,4),
                            C1(10,4),C1(8,2),C1(4,4),C1(0,4)); break;
            case G_SETTILESIZE: fprintf(ModelDump,"    gsDPSetTileSize(%u, %u, %u, %u, %u),\n",
                            C1(24,3),C0(12,12),C0(0,12),C1(12,12),C1(0,12)); break;
            case G_LOADBLOCK: fprintf(ModelDump,"    gsDPLoadBlock(%u, %u, %u, %u, %u),\n",
                            C1(24,3),C0(12,12),C0(0,12),C1(12,12),C1(0,12)); break;
            case G_LOADTLUT: fprintf(ModelDump,"    //gsDPLoadTLUTCmd(%u, %u),\n",C1(24,3),C1(14,10)); break;
            case G_SETCOMBINE: {
                const char* A0First = F3D_CC(CC_PART_A, C0(20, 4));
                const char* B0First = F3D_CC(CC_PART_B, C1(28, 4));
                const char* C0First = F3D_CC(CC_PART_C, C0(15, 5));
                const char* D0First = F3D_CC(CC_PART_D, C1(15, 3));
                // C0(12, 3), C1(12, 3), C0(9, 3),  C1(9, 3)
                const char* AlphaA0First = F3D_AC(CC_PART_A, C0(12, 3));
                const char* AlphaB0First = F3D_AC(CC_PART_B, C1(12, 3));
                const char* AlphaC0First = F3D_AC(CC_PART_C, C0(9, 3));
                const char* AlphaD0First = F3D_AC(CC_PART_D, C1(9, 3));

                // cycle 2
                // C0(5, 4),  C1(24, 4), C0(0, 5),  C1(6, 3)
                const char* A0Second = F3D_CC(CC_PART_A, C0(5, 4));
                const char* B0Second = F3D_CC(CC_PART_B, C1(21, 4));
                const char* C0Second = F3D_CC(CC_PART_C, C0(0, 5));
                const char* D0Second = F3D_CC(CC_PART_D, C1(6, 3));
                // C1(21, 3), C1(3, 3),  C1(18, 3), C1(0, 3),
                const char* AlphaA0Second = F3D_AC(CC_PART_A, C1(21, 3));
                const char* AlphaB0Second = F3D_AC(CC_PART_B, C1(3, 3));
                const char* AlphaC0Second = F3D_AC(CC_PART_C, C1(18, 3));
                const char* AlphaD0Second = F3D_AC(CC_PART_D, C1(0, 3));

                fprintf(ModelDump,"    gsDPSetCombineLERP(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s),\n",
                A0First, B0First, C0First, D0First, AlphaA0First, AlphaB0First, AlphaC0First, AlphaD0First,
                A0Second, B0Second, C0Second, D0Second, AlphaA0Second, AlphaB0Second, AlphaC0Second, AlphaD0Second);
            }
        }
    }
}

void ParseDisplayListRecursive(N64Rom &Rom, u32 DisplayList,
                               std::vector<F3DVertex> &Vertices,
                               std::vector<F3DLight> &Ambients,
                               std::vector<F3DLight> &Lights,
                               std::vector<F3DTexture> &Textures,
                               std::vector<F3DDL> &DLs) {
    u32 Entry = (DisplayList);

    for (const auto &DL : DLs)
        if (DL.DisplayListSeg == DisplayList) return;

    DLs.push_back({ DisplayList, Entry });

    while (true) {
        u32 W0 = Rom.ReadBytes<u32>(Entry);
        u32 W1 = Rom.ReadBytes<u32>(Entry + 4);
        u8 Cmd = W0 >> 24;
        //printf("0x%x Gfxcmd: 0x%x\n", Entry, Cmd);
        ParseRDPTileCommand(Textures, W0, W1, Cmd, false, false, nullptr);
        if (Rom.mMicrocode == UCODE_F3D) {
            if (Cmd == (u8)G_ENDDL) break;
            switch (Cmd) {
                case G_VTX:{ 
                    Vertices.push_back({ (W1), W1, (C0(0, 16)) / 16 });
                } 
                break;
                case G_MOVEMEM: (C0(16, 8) == 0x88 ? Ambients : Lights).push_back({ (W1), W1 }); break;
                case G_DL: {
                    bool Branch = (C0(16, 1) == 0x01);
                    if (ValidateMemAddr(W1)) ParseDisplayListRecursive(Rom, W1, Vertices, Ambients, Lights, Textures, DLs);
                    else printf("DisplayList 0x%08x has an invalid address, ignoring export\n", W1);
                    if (Branch) return;
                    break;
                }
                case (u8)G_TRI1: {
                    if (!Textures.empty()) {
                        F3DTexture &Cur = Textures.back();
                        if (Cur.Texture != 0) {
                            F3DTexture Cpy = Cur;
                            Cpy.Texture = 0;
                            Cpy.TextureSeg = 0;
                            Cpy.Length = 0;
                            Cpy.Width = 0;
                            Cpy.Height = 0;

                            Textures.push_back(Cpy);
                        }
                    }
                    break;
                }
            }
        } else if (Rom.mMicrocode == UCODE_F3DEX2) {
            if (Cmd == (u8)0xdf) break;
            switch (Cmd) {
                case 0x01:{ 
                    Vertices.push_back({ (W1), W1, (C0(12, 8))});
                   // printf("New Vtx: 0x%x\n", W1);
                } 
                break;
                case 0xde: {
                    bool Branch = (C0(16, 1) == 0x01);
                    if (ValidateMemAddr(W1)) ParseDisplayListRecursive(Rom, W1, Vertices, Ambients, Lights, Textures, DLs);
                    else printf("DisplayList 0x%08x has an invalid address, ignoring export\n", W1);
                    if (Branch) return;
                    break;
                }
                case (u8)0x05:
                case (u8)0x06: {
                    if (!Textures.empty()) {
                        F3DTexture &Cur = Textures.back();
                        if (Cur.Texture != 0) {
                            F3DTexture Cpy = Cur;
                            Cpy.Texture = 0;
                            Cpy.TextureSeg = 0;
                            Cpy.Length = 0;
                            Cpy.Width = 0;
                            Cpy.Height = 0;
                            Textures.push_back(Cpy);
                        }
                    }
                    break;
                }
            }
        } else if (Rom.mMicrocode == UCODE_F3DEX) {
            if (Cmd == (u8)G_ENDDL) break;
            switch (Cmd) {
                case G_VTX:{ 
                    Vertices.push_back({ (W1), W1, (C0(10, 6)) });
                } 
                break;
                case G_MOVEMEM: (C0(16, 8) == 0x88 ? Ambients : Lights).push_back({ (W1), W1 }); break;
                
                case G_DL: {
                    bool Branch = (C0(16, 1) == 0x01);
                    if (ValidateMemAddr(W1)) ParseDisplayListRecursive(Rom, W1, Vertices, Ambients, Lights, Textures, DLs);
                    else printf("DisplayList 0x%08x has an invalid address, ignoring export\n", W1);
                    if (Branch) return;
                    break;
                }
                //guess end of texture
                case (u8)(G_IMMFIRST-14):
                case (u8)G_TRI1: {
                    if (!Textures.empty()) {
                        F3DTexture &Cur = Textures.back();
                        if (Cur.Texture != 0) {
                            F3DTexture Cpy = Cur;
                            Cpy.Texture = 0;
                            Cpy.TextureSeg = 0;
                            Cpy.Length = 0;
                            Cpy.Width = 0;
                            Cpy.Height = 0;
                            Textures.push_back(Cpy);
                        }
                    }
                    break;
                }
            }
        }

        Entry += 8;
    }
}

const std::vector<std::pair<uint32_t, std::string>> GeoMacrosF3D = {
    {8192, "G_CULL_BACK"},
    {12288, "G_CULL_BOTH"},
    {4096, "G_CULL_FRONT"},
    {65536, "G_FOG"},
    {131072, "G_LIGHTING"},
    {4, "G_SHADE"},
    {512, "G_SHADING_SMOOTH"},
    {262144, "G_TEXTURE_GEN"},
    {524288, "G_TEXTURE_GEN_LINEAR"},
    {1, "G_ZBUFFER"}
};

const std::vector<std::pair<uint32_t, std::string>> GeoMacrosF3DEX2 = {
    {1024, "G_CULL_BACK"},
    {1536, "G_CULL_BOTH"},
    {512, "G_CULL_FRONT"},
    {65536, "G_FOG"},
    {131072, "G_LIGHTING"},
    {4, "G_SHADE"},
    {2097152, "G_SHADING_SMOOTH"},
    {262144, "G_TEXTURE_GEN"},
    {524288, "G_TEXTURE_GEN_LINEAR"},
    {1, "G_ZBUFFER"}
};

std::string ConvertGeoMode(N64Rom &Rom, uint32_t Flags) {
    std::string Result = "";
    
    std::vector<std::pair<uint32_t, std::string>> macros = GeoMacrosF3D;

    if (Rom.mMicrocode == UCODE_F3DEX2) {
        macros = GeoMacrosF3DEX2;
    }

    for (const auto &macro : macros) {
        uint32_t K = macro.first;
        const std::string &V = macro.second;
        
        if ((Flags & K) == K) {
            Flags = Flags ^ K;
            Result += V + "|";
        }
    }
    
    if (!Result.empty()) {
        Result.pop_back();
    } else {
        Result = "0";
    }
    
    return Result;
}

void ExportModels(N64Rom &Rom, LevelScript &Script, std::string LvlName, u8 Area, const char *FilePath, bool IsActor, Actor *Act) {
    FILE *ModelDump = fopen(FilePath, "w");

    auto GetPlaceHolderName = [LvlName, Area, IsActor, Act](void) {
        return IsActor ? Act->Name : std::format("{}_{}", LvlName, Area);
    };

    std::vector<F3DVertex> Vertices;
    std::vector<F3DLight> Ambients;
    std::vector<F3DLight> Lights;
    std::vector<F3DTexture> Textures;
    std::vector<F3DDL> DLs;

    const std::string TextureFormatNames[] = {
        "rgba",
        "yuv",
        "ci",
        "ia",
        "i"
    };
    if (!IsActor) {
        for (s32 D = 0; D < Script.AreaDatas[Area].DisplayLists.size(); D++) {
            u32 *Data = Script.AreaDatas[Area].DisplayLists.data();
            ParseDisplayListRecursive(Rom, Data[D], Vertices, Ambients, Lights, Textures, DLs);
        }
    } else {
        for (u32 ActorDL : Act->DisplayLists) {
            ParseDisplayListRecursive(Rom, ActorDL, Vertices, Ambients, Lights, Textures, DLs);
        }
    }

    for (const auto &V : Vertices) {
        if (V.Vtx == 0 || !ValidateMemAddr(V.Vtx)) {
            printf("Vertex 0x%08x has an invalid address, ignoring export\n", V.Vtx);
            continue;
        }
        fprintf(ModelDump, "Vtx %s_vertex_0x%x[] = {\n", GetPlaceHolderName().c_str(), V.VtxSeg);
        for (u32 I = 0; I < V.Size; I++) {
            u32 Addr = V.Vtx + I * 16;
            s16 X = Rom.ReadBytes<s16>(Addr + 0);
            s16 Y = Rom.ReadBytes<s16>(Addr + 2);
            s16 Z = Rom.ReadBytes<s16>(Addr + 4);
            s16 TU = Rom.ReadBytes<s16>(Addr + 8);
            s16 TV = Rom.ReadBytes<s16>(Addr + 10);
            u8 R = Rom.ReadBytes<u8>(Addr + 12);
            u8 G = Rom.ReadBytes<u8>(Addr + 13);
            u8 B = Rom.ReadBytes<u8>(Addr + 14);
            u8 A = Rom.ReadBytes<u8>(Addr + 15);

            fprintf(ModelDump, "    {{{ %d, %d, %d }, 0, { %d, %d }, { %u, %u, %u, %u }}},\n",
                    X, Y, Z, TU, TV, R, G, B, A);
        }
        fprintf(ModelDump, "};\n\n");
    }

    for (const auto &A : Ambients) {
        if (A.Light == 0 || !ValidateMemAddr(A.Light)) {
            //printf("Ambient Light 0x%08x has an invalid address, ignoring export\n", A.Light);
            continue;
        }
        fprintf(ModelDump, "Ambient_t %s_light_0x%x[] = {\n", GetPlaceHolderName().c_str(), A.LightSeg);
        u32 Addr = A.Light;
        fprintf(ModelDump, "    { %u, %u, %u}, 0, { %u, %u, %u}, 0\n",
                Rom.ReadBytes<u8>(Addr + 0), Rom.ReadBytes<u8>(Addr + 1), Rom.ReadBytes<u8>(Addr + 2),
                Rom.ReadBytes<u8>(Addr + 4), Rom.ReadBytes<u8>(Addr + 5), Rom.ReadBytes<u8>(Addr + 6));
        fprintf(ModelDump, "};\n\n");
    }

    for (const auto &L : Lights) {
        if (L.Light == 0 || !ValidateMemAddr(L.Light)) {
            //printf("Light 0x%08x has an invalid address, ignoring export\n", L.Light);
            continue;
        }
        fprintf(ModelDump, "Light_t %s_light_0x%x[] = {\n", GetPlaceHolderName().c_str(), L.LightSeg);
        u32 Addr = L.Light;
        fprintf(ModelDump, "    { %u, %u, %u}, 0, { %u, %u, %u}, 0, { %d, %d, %d}, 0\n",
                Rom.ReadBytes<u8>(Addr + 0), Rom.ReadBytes<u8>(Addr + 1), Rom.ReadBytes<u8>(Addr + 2),
                Rom.ReadBytes<u8>(Addr + 4), Rom.ReadBytes<u8>(Addr + 5), Rom.ReadBytes<u8>(Addr + 6),
                Rom.ReadBytes<s8>(Addr + 8), Rom.ReadBytes<s8>(Addr + 9), Rom.ReadBytes<s8>(Addr + 10));
        fprintf(ModelDump, "};\n\n");
    }

    for (auto &T : Textures) {
        if (T.Texture == 0 || !ValidateMemAddr(T.Texture)) {
            if (T.Texture) printf("Texture 0x%08x has an invalid address, ignoring export\n", T.Texture);
            continue;
        }
        fprintf(ModelDump, "u8 %s_texture_0x%x[] = {\n", GetPlaceHolderName().c_str(), T.TextureSeg);
        u16 Width = T.Width ? T.Width : 32;
        u16 Height = T.Height ? T.Height : 32;
        u32 pixels = Width * Height;

        if (T.Length == 0) {
            T.Length = ((pixels + 1) * (u32)T.BitDepth) / 8;
            //printf("Length 0 but has pixels %u\n", pixels);
            //continue;
        }

        std::vector<u8> src(T.Length*2);
        for (u32 j = 0; j < T.Length; j++)
            src[j] = Rom.ReadBytes<u8>(T.Texture + j);

        std::vector<u8> rgba(pixels * 4);

        switch (T.ImgType) {
            case F3D_IMG_RGBA:
            case F3D_IMG_YUV:
                if (T.BitDepth == 16) BinImg::DecodeRGBA16(src.data(), rgba.data(), pixels);
                else if (T.BitDepth == 32) BinImg::DecodeRGBA32(src.data(), rgba.data(), pixels);
                break;
            case F3D_IMG_CI:
                if (T.BitDepth == 4) BinImg::DecodeCI4(src.data(), rgba.data(), pixels, Rom, T.Palette);
                else if (T.BitDepth == 8) BinImg::DecodeCI8(src.data(), rgba.data(), pixels, Rom, T.Palette);
                break;
            case F3D_IMG_I:
                if (T.BitDepth == 4) BinImg::DecodeI4(src.data(), rgba.data(), pixels);
                else if (T.BitDepth == 8) BinImg::DecodeI8(src.data(), rgba.data(), pixels);
                break;
            case F3D_IMG_IA:
                if (T.BitDepth == 4) BinImg::DecodeIA4(src.data(), rgba.data(), pixels);
                else if (T.BitDepth == 8) BinImg::DecodeIA8(src.data(), rgba.data(), pixels);
                else if (T.BitDepth == 16) BinImg::DecodeIA16(src.data(), rgba.data(), pixels);
                break;
            default:
                printf("Unknown format for texture 0x%x, using RGBA\n", T.Texture);
                T.ImgType = F3D_IMG_RGBA;
                break;
        }

        char ImgTypeName[64];
        sprintf(ImgTypeName, "%s%u", TextureFormatNames[T.ImgType].c_str(), T.BitDepth);
        char FileName[256];
        if (IsActor) {
            sprintf(FileName, "output/actors/%s/%s_texture_0x%x.%s.png", Act->Name.c_str(), GetPlaceHolderName().c_str(), T.TextureSeg, ImgTypeName);
        } else {
            sprintf(FileName, "output/levels/%s/%s_texture_0x%x.%s.png", LvlName.c_str(), GetPlaceHolderName().c_str(), T.TextureSeg, ImgTypeName);
        }
        stbi_write_png(FileName, Width, Height, 4, rgba.data(), Width * 4);
        if (IsActor) {
            fprintf(ModelDump, "    #include \"actors/%s/%s_texture_0x%x.%s.inc.c\"\n", Act->Name.c_str(), GetPlaceHolderName().c_str(), T.TextureSeg, ImgTypeName);
        } else {
            fprintf(ModelDump, "    #include \"levels/%s/%s_texture_0x%x.%s.inc.c\"\n", LvlName.c_str(), GetPlaceHolderName().c_str(), T.TextureSeg, ImgTypeName);
        }

        fprintf(ModelDump, "};\n");

        if (T.ImgType == F3D_IMG_CI) {
            fprintf(ModelDump, "u8 %s_texture_0x%x[] = {\n", GetPlaceHolderName().c_str(), T.PaletteSeg);
            fprintf(ModelDump, "    0x00\n");
            fprintf(ModelDump, "};\n");
        }
    }

    for (const auto &DL : DLs) {
        u32 Entry = DL.Entry;
        fprintf(ModelDump, "Gfx %s_displaylist_0x%x[] = {\n", GetPlaceHolderName().c_str(), DL.DisplayListSeg);

        while (true) {
            u32 W0 = Rom.ReadBytes<u32>(Entry);
            u32 W1 = Rom.ReadBytes<u32>(Entry+4);
            u8 Cmd = W0 >> 24;
            bool ShouldEnd = false;
            ParseRDPTileCommand(Textures, W0, W1, Cmd, true, IsActor, Act, ModelDump, LvlName, Area);

            if (Rom.mMicrocode == UCODE_F3D) {
                if (Cmd == (u8)G_ENDDL) { fprintf(ModelDump, "    gsSPEndDisplayList(),\n};\n\n"); break; }

                switch(Cmd) {
                    case G_VTX: fprintf(ModelDump, "    gsSPVertex(%s_vertex_0x%x, %u, %u),\n",
                                    GetPlaceHolderName().c_str(), W1, (C0(0,16))/16, C0(16,4)); break;
                    case (u8)G_TRI1: {
                        u32 NextW0 = Rom.ReadBytes<u32>(Entry+8);
                        u32 NextW1 = Rom.ReadBytes<u32>(Entry+12);
                        u8 NextCmd = NextW0 >> 24;
                        // try optimize
                        if (NextCmd == 0xbf) {
                            fprintf(ModelDump, "    gsSP2Triangles(%u, %u, %u, 0, ", C1(16, 8) / 10, C1(8, 8) / 10, C1(0, 8) / 10);
                            W0 = NextW0;
                            W1 = NextW1;
                            fprintf(ModelDump, "%u, %u, %u, 0),\n", C1(16, 8) / 10, C1(8, 8) / 10, C1(0, 8) / 10);
                            Entry += 8;
                        } else {
                            fprintf(ModelDump, "    gsSP1Triangle(%u, %u, %u, 0),\n", C1(16, 8) / 10, C1(8, 8) / 10, C1(0, 8) / 10);
                        }
                        break;
                    }
                    case (u8)G_CLEARGEOMETRYMODE: fprintf(ModelDump,"    gsSPClearGeometryMode(%s),\n",ConvertGeoMode(Rom, W1).c_str()); break;
                    case (u8)G_SETGEOMETRYMODE: fprintf(ModelDump,"    gsSPSetGeometryMode(%s),\n",ConvertGeoMode(Rom, W1).c_str()); break;
                    case G_MOVEMEM: fprintf(ModelDump,"    gsSPLight(&%s_light_0x%x.col, %u),\n",
                                    GetPlaceHolderName().c_str(), W1, (C0(16,8)==0x88?2:1)); break;
                    case G_DL: {
                        bool Branch = (C0(16, 1) == 0x01);
                        fprintf(ModelDump,"    %s(%s_displaylist_0x%x),\n",
                                Branch ? "gsSPBranchList" : "gsSPDisplayList", GetPlaceHolderName().c_str(), W1);
                        if (Branch) {
                            fprintf(ModelDump, "};\n\n");
                            ShouldEnd = true;
                        }
                        break;
                    }
                    case (u8)G_TEXTURE: fprintf(ModelDump,"    gsSPTexture(%u, %u, %u, %u, %u),\n",C1(16,16),C1(0,16),C0(11,3),C0(8,3),C0(0,8)); break;
                }
            } else if (Rom.mMicrocode == UCODE_F3DEX2) {
                if (Cmd == (u8)0xdf) { fprintf(ModelDump, "    gsSPEndDisplayList(),\n};\n\n"); break; }

                switch(Cmd) {
                    case 0x01: fprintf(ModelDump, "    gsSPVertex(%s_vertex_0x%x, %u, %u),\n", GetPlaceHolderName().c_str(), W1, C0(12, 8), (C0(1, 7) - C0(12, 8))); break;
                    case (u8)0x05: {
                        fprintf(ModelDump, "    gsSP1Triangle(%u, %u, %u, 0),\n", C0(16, 8) / 2, C0(8, 8) / 2, C0(0, 8) / 2);
                        break;
                    }
                    case (u8)0x06: {
                        fprintf(ModelDump, "    gsSP2Triangles(%u, %u, %u, 0, %u, %u, %u, 0),\n", C0(16, 8) / 2, C0(8, 8) / 2, C0(0, 8) / 2, C1(16, 8) / 2, C1(8, 8) / 2, C1(0, 8) / 2);
                        break;
                    }
                    case 0xde: {
                        bool Branch = (C0(16, 1) == 0x01);
                        fprintf(ModelDump,"    %s(%s_displaylist_0x%x),\n",
                                Branch ? "gsSPBranchList" : "gsSPDisplayList", GetPlaceHolderName().c_str(), W1);
                        if (Branch) {
                            fprintf(ModelDump, "};\n\n");
                            ShouldEnd = true;
                        }
                        break;
                    }
                    case (u8)0xd7: fprintf(ModelDump,"    gsSPTexture(%u, %u, %u, %u, %u),\n",C1(16, 16), C1(0, 16), C0(11, 3), C0(8, 3), C0(1, 7)); break;
                    case (u8)0xd9: fprintf(ModelDump,"    gsSPGeometryMode(%s, %s),\n",ConvertGeoMode(Rom, ~C0(0, 24)).c_str(),ConvertGeoMode(Rom, W1).c_str()); break;
                }
            } else if (Rom.mMicrocode == UCODE_F3DEX) {
                if (Cmd == (u8)G_ENDDL) { fprintf(ModelDump, "    gsSPEndDisplayList(),\n};\n\n"); break; }

                switch(Cmd) {
                    case G_VTX: fprintf(ModelDump, "    gsSPVertex(%s_vertex_0x%x, %u, %u),\n",
                                    GetPlaceHolderName().c_str(), W1, (C0(10,6)), C0(16, 8) / 2); break;
                    case (u8)G_TRI1: {
                        fprintf(ModelDump, "    gsSP1Triangle(%u, %u, %u, 0),\n", C1(16, 8) / 2, C1(8, 8) / 2, C1(0, 8) / 2);
                        break;
                    }
                    case (u8)(G_IMMFIRST-14): {
                        fprintf(ModelDump, "    gsSP2Triangles(%u, %u, %u, 0, %u, %u, %u, 0),\n", C0(16, 8) / 2, C0(8, 8) / 2, C0(0, 8) / 2, C1(16, 8) / 2, C1(8, 8) / 2, C1(0, 8) / 2);
                        break;
                    }
                    case (u8)G_CLEARGEOMETRYMODE: fprintf(ModelDump,"    gsSPClearGeometryMode(%s),\n",ConvertGeoMode(Rom, W1).c_str()); break;
                    case (u8)G_SETGEOMETRYMODE: fprintf(ModelDump,"    gsSPSetGeometryMode(%s),\n",ConvertGeoMode(Rom, W1).c_str()); break;
                    case G_MOVEMEM: fprintf(ModelDump,"    gsSPLight(&%s_light_0x%x.col, %u),\n",
                                    GetPlaceHolderName().c_str(), W1, (C0(16,8)==0x88?2:1)); break;
                    case G_DL: {
                        bool Branch = (C0(16, 1) == 0x01);
                        fprintf(ModelDump,"    %s(%s_displaylist_0x%x),\n",
                                Branch ? "gsSPBranchList" : "gsSPDisplayList", GetPlaceHolderName().c_str(), W1);
                        if (Branch) {
                            fprintf(ModelDump, "};\n\n");
                            ShouldEnd = true;
                        }
                        break;
                    }
                    case (u8)G_TEXTURE: fprintf(ModelDump,"    gsSPTexture(%u, %u, %u, %u, %u),\n",C1(16,16),C1(0,16),C0(11,3),C0(8,3),C0(0,8)); break;
                }
            }

            if (ShouldEnd) {
                break;
            }

            Entry += 8;
        }
    }
    
    fclose(ModelDump);
}