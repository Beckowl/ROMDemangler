#pragma once 

#include "LevelScript.h"

struct MovingTextureQC {
    std::string LvlName;
    u16 LvlID;
    u8 Area = 0;
    u32 Index = 0;
    u16 Type = 0;
};

extern std::vector<MovingTextureQC> MovingTextures;

extern void ExportMovTex(N64Rom &Rom, u8 Area, const std::string &LvlName, LevelScript &Script, const char *FilePath);