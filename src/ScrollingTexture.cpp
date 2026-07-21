#include "ScrollingTexture.h"

std::vector<ScrollTexture> ScrollingTextures = {};

u16 GetScrollAxis(u16 Dir) {
    u16 V = Dir & 0xF000;
    switch (V) {
        case 0xA000: return 4;
        case 0x8000: return 5;
        case 0x4000: return 0;
        case 0x2000: return 1;
        case 0x0000: return 2;
        default: return 4;
    }
}

u16 GetScrollType(u16 Dir) {
    u16 V = Dir & 0x0F00;
    switch (V) {
        case 0x000: return 0;
        case 0x100: return 1;
        case 0x200: return 2;
        default: return 0;
    }
}

ScrollTexture ConvertTexScrolls(u32 Bparam, u16 NumVtx, u16 Dir, s16 Speed) {
    ScrollTexture S;
    S.Addr = Bparam;
    S.NumVtx = NumVtx;
    S.Speed = Speed;
    S.Axis = GetScrollAxis(Dir);
    S.Type = GetScrollType(Dir);
    S.Cycle = Dir&0xFF;
    return S;
}