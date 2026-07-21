#pragma once

#include "Global.h"
#include <ultratypes.h>

#include <vector>
#include <cstdio>

extern std::vector<u8> SegmentData[MAX_SEGMENT];
extern u32 SegmentOffsets[MAX_SEGMENT][2];
extern FILE *MapFile;