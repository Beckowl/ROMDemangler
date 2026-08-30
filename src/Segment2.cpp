#include "Segment2.h"
#include "F3D.h"
#include "Memory.h"
#include "N64Rom.h"
#include "stb_image_write.h"
#include <format>
#include <set>

struct TransitionTexData {
    u32 Address = 0;
    const char *Name;
    u32 Width = 0;
    u32 Height = 0;
    u32 Size = 0;
};

const TransitionTexData TransitionData[] = {
    {0x122B8, "0F458", 32, 64, 0x800},
    {0x12AB8, "0FC58", 32, 64, 0x800},
    {0x132B8, "10458", 64, 64, 0x1000},
    {0x142B8, "11458", 32, 64, 0x800}
};

void FindAndLoadSegment2(N64Rom &Rom) {
    u32 Seg2Start = 0;
    u32 Seg2End = 0;
    std::vector<std::pair<u32, u32>> Matches;
    for (u32 i = 0; i < Rom.Size - 16; i += 4) {
        if (Rom.ReadBytes<u32>(i, false) == 0x4D494F30) {
            u32 UncompSize = Rom.ReadBytes<u32>(i + 4, false);
            if (UncompSize >= 32768 && UncompSize <= 131072) {
                Matches.push_back({i, UncompSize});
            }
        }
    }

    u32 WCount = Rom.Size / 4;
    for (const auto &Match : Matches) {
        u32 off = Match.first;
        
        u32 Upper = (off >> 16) & 0xFFFF;
        u32 Lower = off & 0xFFFF;
        if (Lower >= 0x8000) {
            Upper = (Upper + 1) & 0xFFFF;
        }

        bool FoundSeg2 = false;
        for (u32 I = 0; I < WCount; ++I) {
            u32 Word = Rom.ReadBytes<u32>(I * 4, false);
            
            if ((Word >> 26) == 15 && (Word & 0xFFFF) == Upper) {
                u32 ScanStart = (I >= 20) ? (I - 20) : 0;
                u32 ScanEnd = MIN(WCount, I + 20);

                bool HasLower = false;
                bool HasSegLoad = false;

                for (u32 J = ScanStart; J < ScanEnd; ++J) {
                    u32 ScanWord = Rom.ReadBytes<u32>(J * 4, false);
                    u32 ScanOp = ScanWord >> 26;

                    if ((ScanOp == 9 || ScanOp == 13) && (ScanWord & 0xFFFF) == Lower) {
                        HasLower = true;
                    }
                    
                    if ((ScanWord & 0xFFE0FFFF) == 0x24000002 || (ScanWord & 0xFFE0FFFF) == 0x34000002) {
                        HasSegLoad = true;
                    }
                }

                if (HasLower && HasSegLoad) {
                    FoundSeg2 = true;
                    break;
                }
            }
        }

        if (FoundSeg2) {
            Seg2Start = off;
            Seg2End = Rom.Size;
            break;
        }
    }

    if (Seg2Start != 0) {
        printf("Found Segment 2 at 0x%x\n", Seg2Start);
    } else {
        Seg2Start = 0x800000;
    }
    LoadSegment(Rom, 0x02, Seg2Start, Seg2End, true);

    /*std::vector<u8> SrcData(8192);
    for (u32 j = 0; j < 8192; j++) SrcData[j] = Rom.ReadBytes<u8>(0x02000000 + j);

    std::vector<u8> RGBA((16*16) * 4);
    BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), (16*16));
    stbi_write_png("peak.png", 16, 16, 4, RGBA.data(), 64);

    FILE *b = fopen("peak.bin", "wb");
    fwrite(SegmentData[2].data(), 131072, 1, b);
    fclose(b);*/
}

void ExportSeg2Textures(N64Rom &Rom) {
    std::string Seg2Path = "output/textures/segment2/";
    fs::create_directories(Seg2Path);

    printf("Exporting textures\n");

    u32 BaseSegment2Addr = 0x02000000;
    u32 NameOffsets = 0;
    std::set<u32> OverrideAddrs = {0x2600, 0x3200, 0x3A00, 0x3C00, 0x3E00};
    for (u32 Address = 0; Address < 0x4A00; Address += 0x200) {        
        if (OverrideAddrs.count(Address)) {
            NameOffsets += 0x200;
        } else if (Address == 0x4200) {
            NameOffsets += 0xA00;
        }

        u32 PixelCount = 16 * 16;

        std::vector<u8> SrcData(0x200);
        for (u32 I = 0; I < 0x200; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

        u32 Addr = Address + NameOffsets;
        std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", Addr);
        stbi_write_png(FilePath.c_str(), 16, 16, 4, RGBA.data(), 16 * 4);
    }

    NameOffsets = 0xB50;
    for (u32 Address = 0x7000; Address < 0x7600; Address += 0x200) {
        u32 PixelCount = 16 * 16;

        std::vector<u8> SrcData(0x200);
        for (u32 I = 0; I < 0x200; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

        u32 Addr = Address + NameOffsets;
        std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", Addr);
        stbi_write_png(FilePath.c_str(), 16, 16, 4, RGBA.data(), 16 * 4);
    }

    for (u32 Address = 0x7600; Address < 0x7700; Address += 0x80) {
        u32 PixelCount = 8 * 8;

        std::vector<u8> SrcData(0x80);
        for (u32 I = 0; I < 0x80; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

        u32 Addr = Address + NameOffsets;
        std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", Addr);
        stbi_write_png(FilePath.c_str(), 8, 8, 4, RGBA.data(), 8 * 4);
    }

    for (u32 Address = 0x5900; Address < 0x7000; Address += 0x40) {
        u32 PixelCount = 16 * 8;

        std::vector<u8> SrcData(0x40);
        for (u32 I = 0; I < 0x40; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeIA4(SrcData.data(), RGBA.data(), PixelCount);

        std::string FilePath = Seg2Path + std::format("font_graphics.{:05X}.ia4.png", Address);
        stbi_write_png(FilePath.c_str(), 16, 8, 4, RGBA.data(), 16 * 4);
    }

    u32 CreditsOffsets = 0x6200 - 0x4A00;
    for (u32 Address = 0x4A00; Address < 0x5900; Address += 0x80) {
        u32 PixelCount = 8 * 8;

        std::vector<u8> SrcData(0x80);
        for (u32 I = 0; I < 0x80; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

        u32 Addr = Address + CreditsOffsets;
        std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", Addr);
        stbi_write_png(FilePath.c_str(), 8, 8, 4, RGBA.data(), 8 * 4);

    }

    const std::string ShadowNames[] = {"shadow_quarter_circle", "shadow_quarter_square"};
    for (u32 Char = 0; Char < 2; Char++) {
        u32 Address = Char * 0x100 + 0x120B8;
        u32 PixelCount = 16 * 16;

        std::vector<u8> SrcData(0x100);
        for (u32 I = 0; I < 0x100; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeIA8(SrcData.data(), RGBA.data(), PixelCount);

        std::string FilePath = Seg2Path + ShadowNames[Char] + ".ia8.png";
        stbi_write_png(FilePath.c_str(), 16, 16, 4, RGBA.data(), 16 * 4);
    }

    for (const auto &Warp : TransitionData) {
        u32 PixelCount = Warp.Width * Warp.Height;

        std::vector<u8> SrcData(Warp.Size);
        for (u32 I = 0; I < Warp.Size; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Warp.Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeIA8(SrcData.data(), RGBA.data(), PixelCount);

        std::string FilePath = Seg2Path + std::format("segment2.{}.ia8.png", Warp.Name);
        stbi_write_png(FilePath.c_str(), Warp.Width, Warp.Height, 4, RGBA.data(), Warp.Width * 4);
    }

    s32 WaterOffsets = 0x11C58 - 0x14AB8;
    for (u32 Tex = 0; Tex < 5; Tex++) {
        u32 TexLoc = Tex * 0x800 + 0x14AB8;
        u32 PixelCount = 32 * 32;

        std::vector<u8> SrcData(0x800);
        for (u32 I = 0; I < 0x800; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + TexLoc + I);
        std::vector<u8> RGBA(PixelCount * 4);

        if (Tex == 3) {
            BinImg::DecodeIA16(SrcData.data(), RGBA.data(), PixelCount);

            std::string FilePath = Seg2Path + std::format("segment2.{:05X}.ia16.png", (s32)TexLoc + WaterOffsets);
            stbi_write_png(FilePath.c_str(), 32, 32, 4, RGBA.data(), 32 * 4);
        } else {
            BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

            std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", (s32)TexLoc + WaterOffsets);
            stbi_write_png(FilePath.c_str(), 32, 32, 4, RGBA.data(), 32 * 4);
        }
    }
}