#include "Memory.h"
#include "Decompress.h"
#include "Main.h"
#include "json.hpp"
#include <cstdlib>
#include <format>
#include <fstream>
#include <unordered_map>

using json = nlohmann::json;

std::vector<u8> SegmentData[MAX_SEGMENT];
u32 SegmentOffsets[MAX_SEGMENT][2] = {0};
static std::unordered_map<u32, std::string> SymbolMap;

static void LoadSymbolMap(const std::string &Path) {
    std::ifstream File(Path);
    if (!File.is_open()) {
        printf("Failed to find custom symbol file %s, skipping\n", Path.c_str());
        return;
    }

    json Root;
    try {
        File >> Root;
    } catch (const json::parse_error &Err) {
        printf("Failed to parse %s: %s\n", Path.c_str(), Err.what());
        return;
    }

    if (!Root.is_object()) {
        printf("%s does not contain a JSON object at its root, skipping\n", Path.c_str());
        return;
    }

    int Loaded = 0;
    for (auto &[Key, Value] : Root.items()) {
        if (!Value.is_string()) continue;

        char *End = NULL;
        unsigned long Addr = strtoul(Key.c_str(), &End, 16);
        if (End == Key.c_str() || *End != '\0') continue;

        SymbolMap[(u32)Addr] = Value.get<std::string>();
        ++Loaded;
    }

    if (VerbosePrinting) {
        printf("Loaded %d symbols from %s\n", Loaded, Path.c_str());
    }
}

void InitMemoryMap(const std::string &CustomSymbolsPath) {
    SymbolMap.clear();

    LoadSymbolMap("symbolMap.json");

    if (!CustomSymbolsPath.empty()) {
        LoadSymbolMap(CustomSymbolsPath);
    }
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
    if (SegmentData[Bank].empty() && Bank != 0) {
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

CompressionType GetCompressionType(N64Rom &Rom, u32 RomStart) {
    std::string HeaderMagic = {
        static_cast<char>(Rom.ReadBytesPhysical<u8>(RomStart)),
        static_cast<char>(Rom.ReadBytesPhysical<u8>(RomStart + 1)),
        static_cast<char>(Rom.ReadBytesPhysical<u8>(RomStart + 2)),
        static_cast<char>(Rom.ReadBytesPhysical<u8>(RomStart + 3)),
    };

    if (HeaderMagic == "MIO0") {
        return COMPRESSION_MIO0;
    } else if (HeaderMagic.substr(0, 3) == "RNC") {
        return COMPRESSION_RNC;
    } else if (HeaderMagic == "YAY0") {
        return COMPRESSION_YAY0;
    }

    return COMPRESSION_NONE;
}

std::vector<u8> DecompressSegment(N64Rom &Rom, u8 Segment, u32 RomStart, u32 RomEnd) {
    CompressionType Type = GetCompressionType(Rom, RomStart);

    switch (Type) {
        case COMPRESSION_MIO0: return Compression::DecompressMIO0(Rom, RomStart);
        case COMPRESSION_RNC: return Compression::DecompressRNC(Rom, RomStart);
        case COMPRESSION_YAY0: return Compression::DecompressYAY0(Rom, RomStart);
        default:
            printf("Tried to decompress segment 0x%02x which is not compressed\n", Segment);
            u32 Size = RomEnd - RomStart;
            std::vector<u8> SegData;
            SegData.resize(Size);
            Rom.ReadBytesPtr<u8>(RomStart, SegData.data(), Size);
            return SegData;
    }
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