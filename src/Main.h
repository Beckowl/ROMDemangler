#pragma once

#include "F3D.h"
#include "BinImg.h"
#include "N64Rom.h"

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

extern bool VerbosePrinting;
extern bool FoundScriptEntry;
extern bool IgnoreSegment0;
extern std::string ActorsExport;
extern std::string GetLabelFromMap(u32 Address);
extern bool ValidateMemAddr(u32 Address);