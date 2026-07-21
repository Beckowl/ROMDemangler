#pragma once

#include <ultratypes.h>
#include <string>
#include <vector>
#include "N64Rom.h"

class LevelScript;

struct Actor {
    bool IsDL = false;
    std::string Name;
    u32 Addr;     
    std::vector<u32> DisplayLists;
};

extern void ExportActors(N64Rom &Rom, LevelScript &Script);