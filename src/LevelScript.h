#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include <format>

#include "Main.h"
#include "Actor.h"
#include "Decompress.h"

struct AreaData {
    u32 GeoLayout = 0;
    u32 Collision = 0;
    u8 WaterBoxCount = 0;
    std::vector<s16> WaterBoxParams = {};
    std::vector<u32> DisplayLists = {};
};

class LevelScript {
public:
    int StackTop = 0;
    int StackBase = 0;
    std::vector<u32> Stack;
    u8 LevelID = 0;
    struct AreaData AreaDatas[MAX_AREA] = {};
    std::vector<Actor> Actors;
    std::vector<u8> Areas = {};
    u8 CurrArea = 0;
    bool FoundLevel = false;
    std::string Name = "";
    Actor *CurrentActor = nullptr;

    // these are from Quad64
    bool IsPerAreaBank0x0E(void);
    void SetAreaSegmented0x0E(N64Rom &Rom, u8 AreaID);
};

void ExportLevel(N64Rom &Rom, u8 LvlID);