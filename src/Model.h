#pragma once

#include "LevelScript.h"

enum F3DCCPart {
    CC_PART_A,
    CC_PART_B,
    CC_PART_C,
    CC_PART_D
};

void ExportModels(N64Rom &Rom, LevelScript &Script, std::string LvlName, u8 Area, const char *FilePath, bool IsActor = false, Actor *Act = nullptr);