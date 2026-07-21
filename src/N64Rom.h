#pragma once

#include "ultratypes.h"
#include "Global.h"
#include "Memory.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <bit>

enum SM64GameType {
    GT_UNKNOWN,
    GT_ROM_MANAGER,
    GT_EDITOR,
    GT_BBP,
    GT_DECOMP
};

enum N64Microcode {
    UCODE_UNKNOWN = 0,
    UCODE_F3D,
    UCODE_F3DEX,
    UCODE_F3DEX2,
    UCODE_F3DZEX,
};

extern enum SM64GameType GameType;
extern bool ExportSegment0;

class N64Rom {
public:
    FILE *mFile;
    s32 mSize = 0;
    u8 *mData = nullptr;
    u8 *RAM = nullptr;
    std::string mRomInternalName;
    enum N64Microcode mMicrocode = UCODE_UNKNOWN;

    void OpenFile(const char *Path, const char *RAMPath);

    template <typename T>
    T ReadBytesPhysical(s32 Offset) const {
        T Buf{};
        constexpr size_t RealSize = sizeof(T);
        if (Offset + RealSize > mSize) return Buf;
        memcpy(&Buf, mData + Offset, RealSize);
        if constexpr (std::is_integral_v<T> && RealSize > 1) {
            Buf = SwapEndian(Buf);
        }
        return Buf;
    }
    template <typename T>
    T ReadBytes(u32 SegAddr, bool Seg0AsRAM=true) const {
        T Buf{};    
        u8 Bank = SegAddr >> 24;
        u32 Offset = SegAddr & 0xFFFFFF;

        if (Bank > (MAX_SEGMENT - 1)) {
            return 0;
        }

        if (Bank != 0 && !SegmentData[Bank].empty()) {
            const auto &Seg = SegmentData[Bank];
            size_t RealSize = sizeof(T);
            if (Offset + RealSize > Seg.size()) return Buf;

            memcpy(&Buf, Seg.data() + Offset, RealSize);
            if constexpr (std::is_integral_v<T> && sizeof(T) > 1) {
                Buf = SwapEndian(Buf);
            }
            return Buf;
        }

        if (Bank == 0 && ExportSegment0 && Seg0AsRAM) {
            T Buf{};
            constexpr size_t RealSize = sizeof(T);
            memcpy(&Buf, RAM + Offset, RealSize);
            if constexpr (std::is_integral_v<T> && RealSize > 1) {
                Buf = SwapEndian(Buf);
            }
            return Buf;
        }

        return ReadBytesPhysical<T>(Offset);
    }

    template <typename T>
    T *ReadBytesPtr(u32 Offset, T *Buf, u32 Len) {
        size_t RealSize = sizeof(T) * Len;
        if (Offset + RealSize > mSize) return Buf;
        memcpy(Buf, mData + Offset, RealSize);

        if constexpr (std::is_integral_v<T> && sizeof(T) > 1) {
            for (u32 i = 0; i < Len; i++) {
                Buf[i] = SwapEndian(Buf[i]);
            }
        }
        return Buf;
    }

    template <typename T>
    static inline T SwapEndian(T val) {
        return std::byteswap(val);
    }

    void CloseFile(void);
};