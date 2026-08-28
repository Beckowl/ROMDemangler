#include "Memory.h"
#include "Decompress.h"
#include "Main.h"
#include <format>
#include <unordered_map>

std::vector<u8> SegmentData[MAX_SEGMENT];
u32 SegmentOffsets[MAX_SEGMENT][2] = {0};
static std::unordered_map<u32, std::string> SymbolMap;

void InitMemoryMap() {
    SymbolMap.clear();

    FILE *MapFile = fopen("sm64.us.map", "r");
    if (!MapFile) {
        printf("Failed to find SM64 Memory Map, output will not have labels\n");
        return;
    }

    rewind(MapFile);

    char Line[1024];
    while (fgets(Line, sizeof(Line), MapFile)) {
        size_t n = strlen(Line);
        while (n > 0 && (Line[n-1] == '\n' || Line[n-1] == '\r' || isspace((unsigned char)Line[n-1]))) {
            Line[--n] = '\0';
        }

        if (n == 0) continue;

        char *Tokens[64];
        int TCount = 0;
        char *Token = strtok(Line, " \t");
        while (Token && TCount < 64) {
            Tokens[TCount++] = Token;
            Token = strtok(NULL, " \t");
        }

        if (TCount == 0) continue;

        std::string Label = Tokens[TCount - 1];

        if (TCount == 2) {
            for (int i = 0; i < TCount; ++i) {
                char *End = NULL;
                unsigned long Val = strtoul(Tokens[i], &End, 16);

                if (End == Tokens[i] || *End != '\0') continue;

                SymbolMap[(u32)Val] = Label;
            }
        }
    }
    fclose(MapFile);
}

std::string GetLabelFromMap(u32 Address) {
    auto SymIt = SymbolMap.find(Address);
    if (SymIt != SymbolMap.end()) {
        return SymIt->second;
    }

    return std::format("Custom_{:#x}", Address);
}

bool ValidateMemAddr(u32 Address) {
    u32 Bank = Address >> 24;
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

u32 SegmentedToROM(u32 Addr) {
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
        return Compression::DecompressMIO0(Rom, RomStart);
    } else if (HeaderMagic[0] == 'R' && HeaderMagic[1] == 'N' && HeaderMagic[2] == 'C') {
        return Compression::DecompressRNC(Rom, RomStart);
    } else if (HeaderMagic[0] == 'Y' && HeaderMagic[1] == 'A' && HeaderMagic[2] == 'Y' && HeaderMagic[3] == '0') {
        return Compression::DecompressYAY0(Rom, RomStart);
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