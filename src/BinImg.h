#pragma once

#include "F3D.h"
#include "N64Rom.h"

namespace BinImg {
    void DecodeRGBA16(const u8 *Src, u8 *Dst, u32 Pixels);
    void DecodeRGBA32(const u8 *Src, u8 *Dst, u32 Pixels);
    void DecodeCI4(const u8 *Src, u8 *Dst, u32 Pixels, N64Rom &Rom, u32 PalAddr);
    void DecodeCI8(const u8 *Src, u8 *Dst, u32 Pixels, N64Rom &Rom, u32 PalAddr);
    void DecodeI4(const u8 *Src, u8 *Dst, u32 Pixels);
    void DecodeI8(const u8 *Src, u8 *Dst, u32 Pixels);
    void DecodeIA4(const u8 *Src, u8 *Dst, u32 Pixels);
    void DecodeIA8(const u8 *Src, u8 *Dst, u32 Pixels);
    void DecodeIA16(const u8 *Src, u8 *Dst, u32 Pixels);

} // namespace BinImg