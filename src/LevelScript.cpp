#include "LevelScript.h"
#include "Actor.h"
#include "ScrollingTexture.h"
#include "GeoLayout.h"
#include "Collision.h"
#include "MovingTexture.h"
#include "Model.h"
#include "Memory.h"

std::map<u8, std::string> LevelNames = {
    {4, "bbh"},
    {5, "ccm"},
    {6, "castle_inside"},
    {7, "hmc"},
    {8, "ssl"},
    {9, "bob"},
    {10, "sl"},
    {11, "wdw"},
    {12, "jrb"},
    {13, "thi"},
    {14, "ttc"},
    {15, "rr"},
    {16, "castle_grounds"},
    {17, "bitdw"},
    {18, "vcutm"},
    {19, "bitfs"},
    {20, "sa"},
    {21, "bits"},
    {22, "lll"},
    {23, "ddd"},
    {24, "wf"},
    {25, "ending"},
    {26, "castle_courtyard"},
    {27, "pss"},
    {28, "cotmc"},
    {29, "totwc"},
    {30, "bowser_1"},
    {31, "wmotr"},
    {33, "bowser_2"},
    {34, "bowser_3"},
    {36, "ttm"}
};

std::string GetLevelName(u16 ID) {
    if (!LevelNames.contains(ID)) {
        return std::format("ext_level_{}", ID);
    } else {
        return LevelNames[ID];
    }
}

// these are from Quad64
bool LevelScript::IsPerAreaBank0x0E(void) {
    std::vector<u8> &Data = SegmentData[0x19];
    if (Data.empty()) return false;
    if (Data.size() < 0x6000) return false;
    u32 Offset = 0x5FFC;
    return ((Data[0 + Offset] << 24 | Data[1 + Offset] << 16 | Data[2 + Offset] << 8 | Data[3 + Offset]) == 0x4BC9189A);
}
void LevelScript::SetAreaSegmented0x0E(N64Rom &Rom, u8 AreaID) {
    if (!IsPerAreaBank0x0E()) return;

    std::vector<u8> &Data = SegmentData[0x19];

    u32 Start, End;
    u32 Offset = 0x5F00 + (u32)AreaID * 0x10;
    Start = (u32)((Data[Offset] << 24) | (Data[Offset + 1 ] << 16)| (Data[Offset + 2] << 8) | Data[Offset + 3]);
    Offset += 4;
    End = (u32)((Data[Offset] << 24) | (Data[Offset + 1] << 16) | (Data[Offset + 2] << 8) | Data[Offset + 3]);

    LoadSegment(Rom, 0x0E, Start, End);
}

void LevelScript::AddDisplayList(u32 Address, u8 Area) {
    if (!ValidateMemAddr(Address)) return;

    if (CurrentActor) {
        CurrentActor->DisplayLists.push_back(Address);
    } else {
        AreaDatas[Area].DisplayLists.push_back(Address);
    }
}

std::string LvlCommandsName[] = {
    "EXECUTE",
    "EXIT_AND_EXECUTE",
    "EXIT",
    "SLEEP",
    "SLEEP_BEFORE_EXIT",
    "JUMP",
    "JUMP_LINK",
    "RETURN",
    "JUMP_LINK_PUSH_ARG",
    "JUMP_N_TIMES",
    "LOOP_BEGIN",
    "LOOP_UNTIL",
    "JUMP_IF",
    "JUMP_LINK_IF",
    "SKIP_IF",
    "SKIP",
    "SKIP_NOP",
    "CALL",
    "CALL_LOOP",
    "SET_REG",
    "PUSH_POOL",
    "POP_POOL",
    "FIXED_LOAD",
    "LOAD_RAW",
    "LOAD_MIO0",
    "LOAD_MARIO_HEAD",
    "LOAD_MIO0_TEXTURE",
    "INIT_LEVEL",
    "CLEAR_LEVEL",
    "ALLOC_LEVEL_POOL",
    "FREE_LEVEL_POOL",
    "AREA",
    "END_AREA",
    "//LOAD_MODEL_FROM_DL", // so dynos doesnt kill itself
    "LOAD_MODEL_FROM_GEO",
    "CMD23",
    "OBJECT_WITH_ACTS",
    //"OBJECT",
    "MARIO",
    "WARP_NODE",
    "PAINTING_WARP_NODE",
    "INSTANT_WARP",
    "LOAD_AREA",
    "CMD2A",
    "MARIO_POS",
    "CMD2C",
    "CMD2D",
    "TERRAIN",
    "ROOMS",
    "SHOW_DIALOG",
    "TERRAIN_TYPE",
    "NOP",
    "TRANSITION",
    "BLACKOUT",
    "GAMMA",
    "SET_BACKGROUND_MUSIC",
    "SET_MENU_MUSIC",
    "STOP_MUSIC",
    "//MACRO_OBJECTS",
    "CMD3A",
    "WHIRLPOOL",
    "GET_OR_SET",
};

std::string LvlCmdExec(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define EXECUTE(seg, script, scriptEnd, entry) \
    CMD_BBH(0x00, 0x10, seg), \
    CMD_PTR(script), \
    CMD_PTR(scriptEnd), \
    CMD_PTR(entry)
    */

    s16 Segment = Rom.ReadBytes<s16>(Start + 2, false) & 0xff;
    u32 RomStart = Rom.ReadBytes<u32>(Start + 4, false);
    u32 RomEnd = Rom.ReadBytes<u32>(Start + 8, false);
    u32 ScriptEntry = Rom.ReadBytes<u32>(Start + 12, false);

    LoadSegment(Rom, Segment, RomStart, RomEnd);

    u32 Saved = Start + 0x10;
    Script.Stack.push_back(Saved);
	Script.StackTop++;
	Script.Stack.push_back(Script.StackBase);
	Script.StackTop++;
	Script.StackBase=Script.StackTop;
    Start = ScriptEntry;

    if (VerbosePrinting) printf("Jump exec to 0x%x and save 0x%x (0x%x)\n", ScriptEntry, Saved, SegmentedToROM(Saved));

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}, {:#x}",
        Segment, RomStart, RomEnd, ScriptEntry
    );

    return OutArgs;
};

std::string LvlCmdExitAndExec(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define EXIT_AND_EXECUTE(seg, script, scriptEnd, entry) \
    CMD_BBH(0x01, 0x10, seg), \
    CMD_PTR(script), \
    CMD_PTR(scriptEnd), \
    CMD_PTR(entry)
    */

    s16 Segment = Rom.ReadBytes<s16>(Start + 2, false) & 0xff;
    u32 RomStart = Rom.ReadBytes<u32>(Start + 4, false);
    u32 RomEnd = Rom.ReadBytes<u32>(Start + 8, false);
    u32 ScriptEntry = Rom.ReadBytes<u32>(Start + 12, false);

    LoadSegment(Rom, Segment, RomStart, RomEnd);

    Script.StackTop = Script.StackBase;
    Start = (ScriptEntry);

    if (VerbosePrinting) printf("Jump exec to 0x%x\n", ScriptEntry);

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}, {:#x}",
        Segment, RomStart, RomEnd, ScriptEntry
    );

    return "";
};

std::string LvlCmdExit(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define EXIT() \
    CMD_BBH(0x02, 0x04, 0x0000)
    */

    Script.StackTop = Script.StackBase;
    Script.StackTop--;
    Script.StackBase = Script.Stack[Script.StackTop];
    Script.Stack.pop_back();
    Script.StackTop--;
    Start = Script.Stack[Script.StackTop];
    Script.Stack.pop_back();

    if (VerbosePrinting) printf("Exit to 0x%x\n", Start);
    if (Script.FoundLevel) Start = UINT32_MAX;

    return "";
}

std::string LvlCmdSleepBeforeExit(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define SLEEP_BEFORE_EXIT(frames) \
    CMD_BBH(0x04, 0x04, frames)
    */

    s16 Frames = Rom.ReadBytes<s16>(Start + 2, false);

    std::string OutArgs = std::format(
        "{}",
        Frames
    );

    return OutArgs;
};

std::string LvlCmdJump(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define JUMP(target) \
    CMD_BBH(0x05, 0x08, 0x0000), \
    CMD_PTR(target)
    */

    u32 Target = Rom.ReadBytes<u32>(Start + 4, false);

    std::string TargetName = GetLabelFromMap(Target);
    std::string OutArgs = std::format(
        "{}",
        TargetName
    );

    Start = Target;

    if (VerbosePrinting) printf("Jump no link to 0x%x (0x%x)\n", Target, Start);

    return OutArgs;
};


std::string LvlCmdJumpLink(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define JUMP_LINK(target) \
    CMD_BBH(0x06, 0x08, 0x0000), \
    CMD_PTR(target)
    */

    u32 Target = Rom.ReadBytes<u32>(Start + 4, false);

    std::string TargetName = GetLabelFromMap(Target);
    std::string OutArgs = std::format(
        "{}",
        TargetName
    );

    u32 Saved = Start + 0x08;
    if ((Target >> 24) == 0) {
        Start += 8;
    } else {
        Script.Stack.push_back(Saved);
        Script.StackTop += 1;
        Start = Target;
    }

    if (VerbosePrinting) printf("Push Current address & Jump to 0x%x and save 0x%x (0x%x)\n", Target, Saved, SegmentedToROM(Saved));

    return OutArgs;
};

std::string LvlCmdReturn(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define RETURN() \
    CMD_BBH(0x07, 0x04, 0x0000)
    */

    Script.StackTop -= 1;
    Start = Script.Stack[Script.StackTop];
    Script.Stack.pop_back();

    if (VerbosePrinting) printf("Return from Jump to 0x%x\n", Start);

    return "";
};

std::string LvlCmdJumpIf(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define JUMP_IF(op, arg, target) \
    CMD_BBBB(0x0C, 0x0C, op, 0x00), \
    CMD_W(arg), \
    CMD_PTR(target)
    */

    u8 Op = Rom.ReadBytes<u8>(Start + 2, false);
    u32 Arg = Rom.ReadBytes<u32>(Start + 4, false);
    u32 Target = Rom.ReadBytes<u32>(Start + 8, false);

    if (Arg == Script.LevelID) {
        Start = Target;
        Script.FoundLevel = true;
        printf("Found %s at address 0x%x\n", Script.Name.c_str(), Target);
    }

    std::string OutArgs = std::format(
        "{}, {}, {:#x}",
        Op, Arg, Target
    );

    return OutArgs;
};

std::string LvlCmdCallAsm(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define CALL(arg, func) \
    CMD_BBH(0x11, 0x08, arg), \
    CMD_PTR(func)
    */

    s16 Arg = Rom.ReadBytes<s16>(Start + 2, false);
    u32 Func = Rom.ReadBytes<u32>(Start + 4, false);

    std::string FuncName = GetLabelFromMap(Func);
    if (Script.FoundLevel) FuncName = "lvl_init_or_update";
    std::string OutArgs = std::format(
        "{}, /* Func */ {}",
        Arg, FuncName
    );

    return OutArgs;
};

std::string LvlCmdCallLoop(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define CALL_LOOP(arg, func) \
    CMD_BBH(0x12, 0x08, arg), \
    CMD_PTR(func)
    */

    return LvlCmdCallAsm(Rom, Script, Start);
};

std::string LvlCmdLoadRaw(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_RAW(seg, romStart, romEnd) \
    CMD_BBH(0x17, 0x0C, seg), \
    CMD_PTR(romStart), \
    CMD_PTR(romEnd)
    */

    s16 Segment = Rom.ReadBytes<s16>(Start + 2, false) & 0xff;
    u32 RomStart = Rom.ReadBytes<u32>(Start + 4, false);
    u32 RomEnd = Rom.ReadBytes<u32>(Start + 8, false);

    LoadSegment(Rom, Segment, RomStart, RomEnd);

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}",
        Segment, RomStart, RomEnd
    );

    return OutArgs;
};

std::string LvlCmdLoadMio0(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_MIO0(seg, romStart, romEnd) \
    CMD_BBH(0x18, 0x0C, seg), \
    CMD_PTR(romStart), \
    CMD_PTR(romEnd)
    */

    s16 Segment = Rom.ReadBytes<s16>(Start + 2, false) & 0xff;
    u32 RomStart = Rom.ReadBytes<u32>(Start + 4, false);
    u32 RomEnd = Rom.ReadBytes<u32>(Start + 8, false);

    LoadSegment(Rom, Segment, RomStart, RomEnd, true);

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}",
        Segment, RomStart, RomEnd
    );

    return OutArgs;
};

std::string LvlCmdLoadMio0Tex(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_MIO0_TEXTURE(seg, romStart, romEnd) \
    CMD_BBH(0x1A, 0x0C, seg), \
    CMD_PTR(romStart), \
    CMD_PTR(romEnd)
    */

    return LvlCmdLoadMio0(Rom, Script, Start);
};

std::string LvlCmdInitLevel(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define INIT_LEVEL() \
    CMD_BBH(0x1B, 0x04, 0x0000)
    */

    return "";
};

std::string LvlCmdClearLevel(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define CLEAR_LEVEL() \
    CMD_BBH(0x1C, 0x04, 0x0000)
    */

    return "";
};

std::string LvlCmdAllocLevelPool(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define ALLOC_LEVEL_POOL() \
    CMD_BBH(0x1D, 0x04, 0x0000
    */

    return "";
};

std::string LvlCmdFreeLevelPool(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define FREE_LEVEL_POOL() \
    CMD_BBH(0x1E, 0x04, 0x0000)
    */

    return "";
};

std::string LvlCmdStartArea(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define AREA(index, geo) \
    CMD_BBBB(0x1F, 0x08, index, 0), \
    CMD_PTR(geo)    
    */

    u8 Index = Rom.ReadBytes<u8>(Start + 2, false);
    u32 Geo = Rom.ReadBytes<u32>(Start + 4, false);

    std::string AreaGeoName = std::format(
        "{}_area_{}_geo_{:x}",
        Script.Name, Index, Geo
    );
    std::string OutArgs = std::format(
        "/* Index */ {}, /* Geo */ {}",
        Index, AreaGeoName
    );

    if (Index > MAX_AREA) {
        printf("Area Index for command is bigger than MAX_AREA");
        Index &= (MAX_AREA-1);
    }

    Script.AreaDatas[Index].GeoLayout = Geo;
    if (Script.FoundLevel) Script.Areas.push_back(Index);
    Script.CurrArea = Index;

    return OutArgs;
};

std::string LvlCmdEndArea(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define END_AREA() \
    CMD_BBH(0x20, 0x04, 0x0000)
    */

    return "";
};

std::string LvlCmdLoadModelFromDL(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_MODEL_FROM_DL(model, dl, layer) \
    CMD_BBH(0x21, 0x08, ((layer << 12) | model)), \
    CMD_PTR(dl)
    */

    s16 Data = Rom.ReadBytes<s16>(Start + 2, false);
    s16 ModelID = Data & 0xFFF;
    s16 Layer = (Data >> 12) & 0xF;
    u32 DisplayList = Rom.ReadBytes<u32>(Start + 4, false);
    std::string DisplayListName;

    if (DisplayList) {
        Actor NewActor;
        NewActor.IsDL = true;
        DisplayListName = NewActor.Name = std::format("{}_actor_dl_{:x}", Script.Name, DisplayList);
        NewActor.Addr = DisplayList;
        Script.Actors.push_back(NewActor);
    }

    std::string OutArgs = std::format(
        "{:#x}, {}, {:#x}",
        ModelID, DisplayListName, Layer
    );

    return OutArgs;
};

std::string LvlCmdLoadModelFromGeo(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_MODEL_FROM_GEO(model, geo) \
    CMD_BBH(0x22, 0x08, model), \
    CMD_PTR(geo)
    */

    s16 ModelID = Rom.ReadBytes<s16>(Start + 2, false);
    u32 Geo = Rom.ReadBytes<u32>(Start + 4, false);

    std::string GeoName = GetLabelFromMap(Geo);

    if (Geo) {
        Actor NewActor;
        NewActor.IsDL = false;
        bool AddActor = true;
        
        if (GeoName.starts_with("Custom_")) {
            GeoName = std::format("{}_actor_geo_{:x}", Script.Name, Geo);
            if (ActorsExport == "vanilla") {
                AddActor = false;
            }
        } else {
            if (ActorsExport == "custom") {
                AddActor = false;
            }
        }
        
        NewActor.Name = GeoName;
        NewActor.Addr = Geo;
        if (AddActor && (Geo >> 24) != 0x14) Script.Actors.push_back(NewActor);
    }

    std::string OutArgs = std::format(
        "{:#x}, {}",
        ModelID, GeoName
    );

    return OutArgs;
};

std::string LvlCmdPlaceObject(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define OBJECT_WITH_ACTS(model, posX, posY, posZ, angleX, angleY, angleZ, bhvParam, bhv, acts) \
    CMD_BBBB(0x24, 0x18, acts, model), \
    CMD_HHHHHH(posX, posY, posZ, angleX, angleY, angleZ), \
    CMD_W(bhvParam), \
    CMD_PTR(bhv)
    */

    u8 Acts = Rom.ReadBytes<u8>(Start + 2, false);
    u8 ModelID = Rom.ReadBytes<u8>(Start + 3, false);
    s16 PosX = Rom.ReadBytes<s16>(Start + 4, false);
    s16 PosY = Rom.ReadBytes<s16>(Start + 6, false);
    s16 PosZ = Rom.ReadBytes<s16>(Start + 8, false);
    s16 AngleX = Rom.ReadBytes<s16>(Start + 10, false);
    s16 AngleY = Rom.ReadBytes<s16>(Start + 12, false);
    s16 AngleZ = Rom.ReadBytes<s16>(Start + 14, false);
    u32 BhvParam = Rom.ReadBytes<u32>(Start + 16, false);
    u32 Bhv = Rom.ReadBytes<u32>(Start + 20, false);

    std::string BhvName = GetLabelFromMap(Bhv);
    if (GameType == GT_ROM_MANAGER || GameType == GT_EDITOR) {
        if (BhvName == "editor_Scroll_Texture2") {
            BhvName = "editor_Scroll_Texture";
        }

        if (BhvName == "RM_Scroll_Texture" || BhvName == "editor_Scroll_Texture") {
            u16 NumVerts = PosX;
            u16 Dir = PosY;
            s16 Speed = PosZ;

            ScrollTexture Scroll = ConvertTexScrolls(BhvParam, NumVerts, Dir, Speed);
            Scroll.LvlName = Script.Name;
            Scroll.Area = Script.CurrArea;
            Scroll.Index = ScrollingTextures.size();
            ScrollingTextures.push_back(Scroll);

            std::string OutArgs = std::format(
                "/* Model */ 0x0, /* Speed */ {}, /* Axis */ {}, /* VCount */ {}, 0, /* Type */ {}, /* Cycle */ {}, /* Index */ {:#x}, {}, /* Act */ {}",
                Scroll.Speed, Scroll.Axis, Scroll.NumVtx,  Scroll.Type, Scroll.Cycle, Scroll.Index, BhvName, Acts
            );
            return OutArgs;
        }
    }

    std::string OutArgs = std::format(
        "/* Model */ {:#x}, /* Pos */ {}, {}, {}, /* Angle */ {}, {}, {}, /* Param */ {:#x}, /* Behavior */ {}, /* Act */ {}",
        ModelID, PosX, PosY, PosZ, AngleX, AngleY, AngleZ, BhvParam, BhvName, Acts
    );

    return OutArgs;
};

std::string LvlCmdPlaceMario(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define MARIO(model, bhvArg, bhv) \
    CMD_BBBB(0x25, 0x0C, 0x00, model), \
    CMD_W(bhvArg), \
    CMD_PTR(bhv)
    */
    
    u8 ModelID = Rom.ReadBytes<u8>(Start + 3, false);
    u32 BhvArg = Rom.ReadBytes<u32>(Start + 4, false);
    u32 Bhv = Rom.ReadBytes<u32>(Start + 8, false);

    std::string BhvName = "bhvMario";
    std::string OutArgs = std::format(
        "/* Model */ {:#x}, /* Param */ {:#x}, /* Behavior */ {}",
        ModelID, BhvArg, BhvName
    );

    return OutArgs;
};

std::string LvlCmdWarpNode(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define WARP_NODE(id, destLevel, destArea, destNode, flags) \
    CMD_BBBB(0x26, 0x08, id, destLevel), \
    CMD_BBBB(destArea, destNode, flags, 0x00)
    */

    u8 Id = Rom.ReadBytes<u8>(Start + 2, false);
    u8 DestLevel = Rom.ReadBytes<u8>(Start + 3, false);
    u8 DestArea = Rom.ReadBytes<u8>(Start + 4, false);
    u8 DestNode = Rom.ReadBytes<u8>(Start + 5, false);
    u8 Flags = Rom.ReadBytes<u8>(Start + 6, false);

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}, {:#x}, {:#x}",
        Id, DestLevel, DestArea, DestNode, Flags
    );

    return OutArgs;
};


std::string LvlCmdPlaceInstaWarp(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define INSTANT_WARP(index, destArea, displaceX, displaceY, displaceZ) \
    CMD_BBBB(0x28, 0x0C, index, destArea), \
    CMD_HH(displaceX, displaceY), \
    CMD_HH(displaceZ, 0x0000)
    */

    u8 Id = Rom.ReadBytes<u8>(Start + 2, false);
    u8 DestArea = Rom.ReadBytes<u8>(Start + 3, false);
    s16 DispX = Rom.ReadBytes<s16>(Start + 4, false);
    s16 DispY = Rom.ReadBytes<s16>(Start + 6, false);
    s16 DispZ = Rom.ReadBytes<s16>(Start + 8, false);

    std::string OutArgs = std::format(
        "{}, {}, {}, {}, {}",
        Id, DestArea, DispX, DispY, DispZ
    );

    return OutArgs;
};

std::string LvlCmdSetMarioPos(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define MARIO_POS(area, yaw, posX, posY, posZ) \
    CMD_BBBB(0x2B, 0x0C, area, 0x00), \
    CMD_HH(yaw, posX), \
    CMD_HH(posY, posZ)
    */

    u8 Area = Rom.ReadBytes<u8>(Start + 2, false);
    s16 Yaw = Rom.ReadBytes<s16>(Start + 4, false);
    s16 PosX = Rom.ReadBytes<s16>(Start + 6, false);
    s16 PosY = Rom.ReadBytes<s16>(Start + 8, false);
    s16 PosZ = Rom.ReadBytes<s16>(Start + 10, false);

    std::string OutArgs = std::format(
        "{}, {}, {}, {}, {}",
        Area, Yaw, PosX, PosY, PosZ
    );

    return OutArgs;
};

std::string LvlCmdSetTerrain(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define TERRAIN(terrainData) \
    CMD_BBH(0x2E, 0x08, 0x0000), \
    CMD_PTR(terrainData)
    */

    u32 Collision = Rom.ReadBytes<u32>(Start + 4, false);

    std::string AreaColName = std::format(
        "{}_area_{}_collision",
        Script.Name, Script.CurrArea
    );
    std::string OutArgs = std::format(
        "/* Col */ {}",
        AreaColName
    );

    Script.AreaDatas[Script.CurrArea].Collision = Collision;

    return OutArgs;
};

std::string LvlCmdShowDialog(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define SHOW_DIALOG(index, dialogId) \
    CMD_BBBB(0x30, 0x04, index, dialogId)
    */

    u8 Index = Rom.ReadBytes<u8>(Start + 2, false);
    u8 DialogID = Rom.ReadBytes<u8>(Start + 3, false);

    std::string OutArgs = std::format(
        "{}, {}",
        Index, DialogID
    );

    return OutArgs;
};

std::string LvlCmdSetTerrainType(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define TERRAIN_TYPE(terrainType) \
    CMD_BBH(0x31, 0x04, terrainType)
    */

    s16 Type = Rom.ReadBytes<s16>(Start + 2, false);

    std::string OutArgs = std::format(
        "{:#x}",
        Type
    );

    return OutArgs;
};

std::string LvlCmdSetMusic(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define SET_BACKGROUND_MUSIC(settingsPreset, seq) \
    CMD_BBH(0x36, 0x08, settingsPreset), \
    CMD_HH(seq, 0x0000)
    */

    s16 Preset = Rom.ReadBytes<s16>(Start + 2, false);
    s16 Sequence = Rom.ReadBytes<s16>(Start + 4, false);

    std::string OutArgs = std::format(
        "{:#x}, {:#x}",
        Preset, Sequence
    );

    return OutArgs;
};

std::string LvlCmdStub(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    return "";
};

std::string (*LvlCommandsFunctions[])(N64Rom &Rom, LevelScript &Script, u32 &Start) = {
    LvlCmdExec,             LvlCmdExitAndExec,      LvlCmdExit,             LvlCmdSleepBeforeExit,
    LvlCmdSleepBeforeExit,  LvlCmdJump,             LvlCmdJumpLink,         LvlCmdReturn,
    (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),
    LvlCmdJumpIf,           (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),
    (LvlCmdStub),           LvlCmdCallAsm,          LvlCmdCallLoop,         (LvlCmdStub),
    (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),           LvlCmdLoadRaw,
    LvlCmdLoadMio0,         (LvlCmdStub),           LvlCmdLoadMio0Tex,      LvlCmdInitLevel,
    LvlCmdClearLevel,       LvlCmdAllocLevelPool,   LvlCmdFreeLevelPool,    LvlCmdStartArea,
    LvlCmdEndArea,          LvlCmdLoadModelFromDL,  LvlCmdLoadModelFromGeo, (LvlCmdStub),
    LvlCmdPlaceObject,      LvlCmdPlaceMario,       LvlCmdWarpNode,         (LvlCmdStub),
    LvlCmdPlaceInstaWarp,   (LvlCmdStub),           (LvlCmdStub),           LvlCmdSetMarioPos,
    (LvlCmdStub),           (LvlCmdStub),           LvlCmdSetTerrain,       (LvlCmdStub),
    LvlCmdShowDialog,       LvlCmdSetTerrainType,   (LvlCmdStub),           (LvlCmdStub),
    (LvlCmdStub),           (LvlCmdStub),           LvlCmdSetMusic,         (LvlCmdStub),
    (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),
    (LvlCmdStub),
};

bool IsJumpLvlCmd(u8 Cmd) {
    return Cmd == 0x00 || Cmd == 0x01 || Cmd == 0x02 || Cmd == 0x05 || Cmd == 0x06 || Cmd == 0x07 || Cmd == 0x0C || Cmd == 0x10 || Cmd == 0x09;
}

void ExportAreas(N64Rom &Rom, LevelScript &Script, const std::string &LvlName) {
    std::string AreasPath = "output/levels/"+LvlName+"/areas";
    fs::create_directories(AreasPath);
    for (auto &I : Script.Areas) {
        u32 GeoSegAddr = Script.AreaDatas[I].GeoLayout;
        u32 ColSegAddr = Script.AreaDatas[I].Collision;

        Script.SetAreaSegmented0x0E(Rom, I);

        char AreaIDStr[20];
        snprintf(AreaIDStr, 20, "%u", I);
        std::string AreaStrNum = AreasPath+"/"+AreaIDStr;
        fs::create_directories(AreaStrNum);
        std::string GeoDumpPath = AreaStrNum + "/geo.inc.c";
        ExportGeolayout(Rom, I, LvlName, GeoSegAddr, GeoSegAddr, Script, GeoDumpPath.c_str());
        std::string ColDumpPath = AreaStrNum + "/collision.inc.c";
        ExportCollision(Rom, I, LvlName, ColSegAddr, ColSegAddr, Script, ColDumpPath.c_str());
        if (GameType == GT_ROM_MANAGER || GameType == GT_EDITOR) {
            std::string MovTextDumpPath = AreaStrNum + "/movtext.inc.c";
            ExportMovTex(Rom, I, LvlName, Script, MovTextDumpPath.c_str());
        }
        std::string ModelDumpPath = AreaStrNum + "/model.inc.c";
        ExportModels(Rom, Script, LvlName, I, ModelDumpPath.c_str());

        printf("%s Area %u done\n", LvlName.c_str(), I);
    }
}

void ExportLevel(N64Rom &Rom, u8 LvlID) {
    std::string LvlName = GetLevelName(LvlID);
    fs::create_directories("output/levels");
    fs::create_directories("output/levels/" + LvlName);
    std::string ScriptPath = "output/levels/" + LvlName + "/script.c";
    FILE *ScriptDump = fopen(ScriptPath.c_str(), "w");
    LevelScript Script;
    Script.LevelID = LvlID;
    Script.Name = LvlName;

    for (auto &SegOff : SegmentOffsets) {
        SegOff[0] = 0;
        SegOff[1] = 0;
    }
    for (auto &SegData : SegmentData) {
        SegData.clear();
    }

    //b'\x1b\x04\x00\x00\x03\x04\x00\x024\x04\x00\x00'
    const u8 Pattern[] = {0x1b, 0x04, 0x00, 0x00, 0x03, 0x04, 0x00, 0x02, 0x34, 0x04, 0x00, 0x00};
    size_t PatternLen = sizeof(Pattern);
    u32 Entry = 0;
    u8* Start = Rom.mData;
    u8* End = Rom.mData + Rom.mSize;
    u8* Found = std::search(Start, End, Pattern, Pattern + PatternLen);
    if (Found != End) {
        Entry = (u32)(Found - Start);
        if (!FoundScriptEntry) printf("Script Entry found at address: 0x%x\n", Entry);
        FoundScriptEntry = true;
    } else {
        printf("No Script Entries could be found.\n");
        exit(1);
    }
    SegmentOffsets[0x10][0] = Entry;

    auto ShouldPrintCmd = [&](u8 Cmd) {
        if (!Script.FoundLevel) {
            return false;
        }
        if (IsJumpLvlCmd(Cmd)) {
            if (Cmd != 0x02) {
                return false;
            }
        }
        return true;
    };

    std::string EntryName = "level_" + LvlName + "_entry";
    fprintf(ScriptDump, "const LevelScript %s[] = {\n", EntryName.c_str());
    printf("Exporting %s\n", LvlName.c_str());
    while (true) {
        u8 Cmd = Rom.ReadBytes<u8>(Entry, false);
        if (Cmd > 60) {
            printf("Unknown LevelScript Command at address 0x%x\n", Entry);
            break;
        }
        //printf("0x%x CMD: 0x%x\n", Entry, Cmd);
        u8 Len = Rom.ReadBytes<u8>(Entry + 1, false);
        bool ShouldPrint = ShouldPrintCmd(Cmd);
        if (ShouldPrint) fprintf(ScriptDump, "    %s(", LvlCommandsName[Cmd].c_str());
        if (LvlCommandsFunctions[Cmd]) {
            std::string Args = LvlCommandsFunctions[Cmd](Rom, Script, Entry);
            if (ShouldPrint) fprintf(ScriptDump, "%s", Args.c_str());
        } else {
            break;
        }
        if (ShouldPrint) fprintf(ScriptDump, "),\n");
        if (Entry == UINT32_MAX) {
            if (VerbosePrinting) printf("Exit Level\n");
            break;
        }
        if ((Cmd == 0x0C && !Script.FoundLevel) || (Cmd != 0x06 && Cmd != 0x02 && Cmd != 0x07 && Cmd != 0x0C && Cmd != 0x00 && Cmd != 0x01 && Cmd != 0x05)) {
            Entry += Len;
        }
    }
    fprintf(ScriptDump, "};\n");
    fclose(ScriptDump);

    ExportAreas(Rom, Script, LvlName);
    if (ActorsExport != "none") ExportActors(Rom, Script);
}
