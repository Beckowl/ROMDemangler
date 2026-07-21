#pragma once

#include <ultratypes.h>
#include <string>
#include <vector>

struct ScrollTexture {
    std::string LvlName;
    u8 Area = 0;
    u32 Index = 0;
    u32 Addr = 0;
    u16 NumVtx = 0;
    s16 Speed = 0;
    u16 Axis = 0;
    u16 Type = 0;
    u8 Cycle = 0;
};

extern std::vector<ScrollTexture> ScrollingTextures;

extern u16 GetScrollAxis(u16 Dir);
extern u16 GetScrollType(u16 Dir);
extern ScrollTexture ConvertTexScrolls(u32 Bparam, u16 NumVtx, u16 Dir, s16 Speed);