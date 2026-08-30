#pragma once

#include "F3D.h"
#include "BinImg.h"
#include "N64Rom.h"
#include <filesystem>

namespace fs = std::filesystem;

extern bool SoundExport;
extern bool TweakExport;
extern bool VerbosePrinting;
extern u32 FoundScriptEntry;
extern bool IgnoreSegment0;
extern bool CollisionFix;
extern bool SkyboxExport;
extern bool TextExport;
extern std::string ActorsExport;