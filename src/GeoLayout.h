#pragma once

#include "LevelScript.h"

extern void ExportGeolayout(N64Rom &Rom, u8 Area, const std::string &LvlName, u32 SegAddr, u32 Entry, LevelScript &Script, const char *FilePath);
extern u8 GetGeolayoutCmdSize(N64Rom &Rom, u32 Entry);