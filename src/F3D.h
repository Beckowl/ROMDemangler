#pragma once

#include "gbi.h"

typedef struct {
    u32 Vtx;
    u32 VtxSeg;
    u32 Size;
} F3DVertex;

typedef struct {
    u32 Light;
    u32 LightSeg;
} F3DLight;

typedef enum {
    F3D_IMG_RGBA,
    F3D_IMG_YUV,
    F3D_IMG_CI,
    F3D_IMG_IA,
    F3D_IMG_I
} F3DImageType;

typedef struct {
    u32 Texture;
    u32 TextureSeg;
    u32 Length;
    u16 Width;
    u16 Height;
    F3DImageType ImgType;
    u8 BitDepth;
    u32 Palette;
    u32 PaletteSeg;
    u8 Tile;
} F3DTexture;

typedef struct {
    u32 DisplayListSeg;
    u32 Entry;
} F3DDL;

#define C0(Pos, Width) ((W0 >> (Pos)) & ((1U << Width) - 1))
#define C1(Pos, Width) ((W1 >> (Pos)) & ((1U << Width) - 1))