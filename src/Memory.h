#pragma once

#include "Global.h"
#include <ultratypes.h>

#include <vector>
#include <cstdio>
#include <string>
#include "N64Rom.h"

extern std::vector<u8> SegmentData[MAX_SEGMENT];
extern u32 SegmentOffsets[MAX_SEGMENT][2];
extern FILE *MapFile;

extern std::string GetLabelFromMap(u32 Address);
extern bool ValidateMemAddr(u32 Address);
extern u32 SegmentedToROM(u32 Addr);
extern std::vector<u8> DecompressSegment(N64Rom &Rom, u8 Segment, u32 RomStart, u32 RomEnd);
extern void LoadSegment(N64Rom &Rom, u8 Segment, u32 RomStart, u32 RomEnd, bool Decompress = false);