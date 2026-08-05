#pragma once

#include "ultratypes.h"
#include "Global.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <bit>

enum SM64GameTypeID {
    GT_UNKNOWN,
    GT_ROM_MANAGER,
    GT_EDITOR,
    GT_BBP,
    GT_DECOMP,
    GT_HACKER
};

struct SM64GameType {
    u8 ID = GT_UNKNOWN;

    void SetID(u8 NewID) {
        ID = NewID;
    }
    bool IsOldBinary(void) {
        return (ID == GT_ROM_MANAGER || ID == GT_EDITOR);
    }
    bool IsNewBinary(void) {
        return (ID == GT_BBP);
    }
    bool IsDecomp(void) {
        return (ID == GT_DECOMP || ID == GT_HACKER);
    }
    bool IsHacker(void) {
        return (ID == GT_HACKER);
    }
    u8 GetID(void) {
        return ID;
    }
};

enum N64Microcode {
    UCODE_UNKNOWN = 0,
    UCODE_F3D,
    UCODE_F3DEX,
    UCODE_F3DEX2,
    UCODE_F3DZEX,
};

extern SM64GameType GameType;
extern bool ExportSegment0;

extern std::vector<u8> SegmentData[MAX_SEGMENT];
extern u32 SegmentOffsets[MAX_SEGMENT][2];

class N64Rom {
public:
    FILE *File;
    size_t Size = 0;
    u8 *Data = nullptr;
    u8 *RAM = nullptr;
    std::string ROMInternalName;
    enum N64Microcode Microcode = UCODE_UNKNOWN;

    void OpenFile(const char *Path, const char *RAMPath);

    template <typename T>
    T ReadBytesPhysical(s32 Offset) {
        T Buf{};
        size_t RealSize = sizeof(T);
        if (Offset + RealSize > Size) return Buf;
        memcpy(&Buf, Data + Offset, RealSize);
        if (RealSize > 1) {
            Buf = SwapEndian(Buf);
        }
        return Buf;
    }
    template <typename T>
    T ReadBytes(u32 SegAddr, bool Seg0AsRAM=true) {
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
            if (RealSize > 1) {
                Buf = SwapEndian(Buf);
            }
            return Buf;
        }

        if (Bank == 0 && ExportSegment0 && Seg0AsRAM) {
            T Buf{};
            size_t RealSize = sizeof(T);
            memcpy(&Buf, RAM + Offset, RealSize);
            if (RealSize > 1) {
                Buf = SwapEndian(Buf);
            }
            return Buf;
        }

        return ReadBytesPhysical<T>(Offset);
    }

    template <typename T>
    T *ReadBytesPtr(u32 Offset, T *Buf, u32 Len) {
        size_t RealSize = sizeof(T) * Len;
        if (Offset + RealSize > Size) return Buf;
        memcpy(Buf, Data + Offset, RealSize);

        if (sizeof(T) > 1) {
            for (u32 i = 0; i < Len; i++) {
                Buf[i] = SwapEndian(Buf[i]);
            }
        }
        return Buf;
    }

    template <typename T>
    static inline T SwapEndian(T Val) {
        if constexpr(sizeof(T) == 1) {
            return Val;
        } else if constexpr(std::is_integral_v<T>) {
            return std::byteswap(Val);
        } else if constexpr(std::is_floating_point_v<T>) {
            if constexpr(sizeof(T) == 4) {
                auto Bits = std::bit_cast<u32>(Val);
                return std::bit_cast<T>(std::byteswap(Bits));
            } 
            else if constexpr(sizeof(T) == 8) {
                auto Bits = std::bit_cast<u64>(Val);
                return std::bit_cast<T>(std::byteswap(Bits));
            }
        }
        return Val;
    }

    void CloseFile(void);
};