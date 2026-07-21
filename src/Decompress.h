#pragma once

#include <vector>
#include <ultratypes.h>
#include "N64Rom.h"

extern std::vector<u8> DecompressMIO0(N64Rom &Rom, u32 RomStart);
extern std::vector<u8> DecompressYAY0(N64Rom &Rom, u32 RomStart);
extern std::vector<u8> DecompressRNC(N64Rom &Rom, u32 RomStart);