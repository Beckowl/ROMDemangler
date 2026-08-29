#pragma once

#include "Global.h"
#include "ultratypes.h"

#include <vector>
#include <cstdio>
#include <string>
#include "N64Rom.h"

typedef enum {
    COMPRESSION_NONE,
    COMPRESSION_MIO0,
    COMPRESSION_RNC,
    COMPRESSION_YAY0,
} CompressionType;

extern std::vector<u8> SegmentData[MAX_SEGMENT];
extern u32 SegmentOffsets[MAX_SEGMENT][2];

extern void InitMemoryMap(const std::string &CustomSymbolsPath = "");
extern std::string GetLabelFromMap(u32 Address);
extern bool ValidateMemAddr(u32 Address);
extern u32 SegmentedToROM(u32 Addr);
extern CompressionType GetCompressionType(N64Rom &Rom, u32 romStart);
extern std::vector<u8> DecompressSegment(N64Rom &Rom, u8 Segment, u32 RomStart, u32 RomEnd);
extern void LoadSegment(N64Rom &Rom, u8 Segment, u32 RomStart, u32 RomEnd, bool Decompress = false);