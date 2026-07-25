#include "Memory.h"
#include "Decompress.h"
#include "Main.h"
#include <format>

std::vector<u8> SegmentData[MAX_SEGMENT];
u32 SegmentOffsets[MAX_SEGMENT][2] = {0};
FILE *MapFile = fopen("sm64.us.map", "r");

std::string GetLabelFromMap(u32 Address) {
    rewind(MapFile);

    char Line[1024];
    while (fgets(Line, sizeof(Line), MapFile)) {
        size_t n = strlen(Line);
        while (n > 0 && (Line[n-1] == '\n' || Line[n-1] == '\r' || isspace((unsigned char)Line[n-1]))) {
            Line[--n] = '\0';
        }

        char Tmp[1024];
        strncpy(Tmp, Line, sizeof(Tmp));
        Tmp[sizeof(Tmp)-1] = '\0';

        char *Tokens[64];
        int TCount = 0;
        char *Token = strtok(Tmp, " \t");
        while (Token && TCount < (int)(sizeof(Tokens)/sizeof(Tokens[0]))) {
            Tokens[TCount++] = Token;
            Token = strtok(NULL, " \t");
        }

        if (TCount == 0) continue;

        for (int i = 0; i < TCount; ++i) {
            char *End = NULL;
            unsigned long Val = strtoul(Tokens[i], &End, 16);
            if (End == Tokens[i]) continue;
            if (*End != '\0') continue;

            if ((u32)Val == Address) {
                char *Label = Tokens[TCount - 1];
                return std::string(Label);
            }
        }
    }

    std::string Buf = std::format(
        "Custom_{:#x}", 
        Address
    );
    return Buf;
}

bool ValidateMemAddr(u32 Address) {
    s32 Bank = Address >> 24;
    if (Bank > (MAX_SEGMENT - 1)) {
        return false;
    }
    if (!Address) {
        return false;
    }
    if (IgnoreSegment0 && Bank == 0) {
        return false;
    }
    return true;
}

u32  SegmentedToROM(u32 Addr) {
    u8 Bank = Addr >> 24;
    u32 Offset = Addr & 0xFFFFFF;
    u32 Start = SegmentOffsets[Bank][0];
    u32 End = SegmentOffsets[Bank][1];
    u32 Result = Start + Offset;

    return Result;
}

std::vector<u8> DecompressSegment(N64Rom &Rom, u8 Segment, u32 RomStart, u32 RomEnd) {
    u8 HeaderMagic[4] = {
        Rom.ReadBytesPhysical<u8>(RomStart),
        Rom.ReadBytesPhysical<u8>(RomStart+1),
        Rom.ReadBytesPhysical<u8>(RomStart+2),
        Rom.ReadBytesPhysical<u8>(RomStart+3),
    };

    if (HeaderMagic[0] == 'M' && HeaderMagic[1] == 'I' && HeaderMagic[2] == 'O' && HeaderMagic[3] == '0') {
        return DecompressMIO0(Rom, RomStart);
    } else if (HeaderMagic[0] == 'R' && HeaderMagic[1] == 'N' && HeaderMagic[2] == 'C') {
        return DecompressRNC(Rom, RomStart);
    } else if (HeaderMagic[0] == 'Y' && HeaderMagic[1] == 'A' && HeaderMagic[2] == 'Y' && HeaderMagic[3] == '0') {
            return DecompressYAY0(Rom, RomStart);
    } else {
        printf("Tried to decompress segment 0x%02x which is not compressed\n", Segment);
        u32 Size = RomEnd - RomStart;
        std::vector<u8> SegData;
        SegData.resize(Size);
        Rom.ReadBytesPtr<u8>(RomStart, SegData.data(), Size);
        return SegData;
    }

    return {};
}

void LoadSegment(N64Rom &Rom, u8 Segment, u32 RomStart, u32 RomEnd, bool Decompress) {
    SegmentOffsets[Segment][0] = RomStart;
    SegmentOffsets[Segment][1] = RomEnd;

    if (RomEnd <= RomStart) {
        SegmentData[Segment].clear();
        return;
    }

    if (Decompress) {
        SegmentData[Segment] = DecompressSegment(Rom, Segment, RomStart, RomEnd);

        if (VerbosePrinting) {
            printf("Loaded Compressed Segment 0x%02X: 0x%08X - 0x%08X\n", Segment, RomStart, RomEnd);
        }
    } else {
        u32 Size = RomEnd - RomStart;
        SegmentData[Segment].resize(Size);
        Rom.ReadBytesPtr<u8>(RomStart, SegmentData[Segment].data(), Size);
        if (VerbosePrinting) {
            printf("Loaded Segment 0x%02X: 0x%08X - 0x%08X\n", Segment, RomStart, RomEnd);
        }
    }
}