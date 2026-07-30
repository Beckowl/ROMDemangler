#include "Tweak.h"

struct StarPosition {
    std::string Name;
    u32 Address = 0;
    bool isS16 = false;
};

struct ROMTweak {
    std::string Name;
    u32 Address = 0;
    u8 Size = 0;
};

struct BehaviorTweak : public ROMTweak {
    bool Dialog = false;
};

struct LevelTweak : public ROMTweak {
};

StarPosition StarPositions[] = {
    {"KoopaBobStarPos", 0xED868, true},
    {"KoopaThiStarPos", 0xED878, true},
    {"KingBobombStarPos", 0x1204F00, false},
    {"KingWhompStarPos", 0x1204F10, false},
    {"EyerockStarPos", 0x1204F20, false},
    {"BigBullyStarPos", 0x1204F30, false},
    {"ChillBullyStarPos", 0x1204F40, false},
    {"BigPiranhasStarPos", 0x1204F50, false},
    {"TuxieMotherStarPos", 0x1204F60, false},
    {"WigglerStarPos", 0x1204F70, false},
    {"PssSlideStarPos", 0x1204F80, false},
    {"RacingPenguinStarPos", 0x1204F90, false},
    {"TreasureChestStarPos", 0x1204FA0, false},
    {"GhostHuntBooStarPos", 0x1204FAC, false},
    {"MerryGoRoundStarPos", 0x1204FB8, false},
    {"KleptoStarPos", 0x1204FC4, false},
    {"MrIStarPos", 0x1204FD0, false},
    {"BalconyBooStarPos", 0x1204FDC, false},
    {"BigBullyTrioStarPos", 0x1204FE4, false},
};
BehaviorTweak BehaviorTweakValues[] = {
    {"ToadStar1Requirement", 0x3199B, 1, false},
    {"ToadStar2Requirement", 0x319CF, 1, false},

    {"ToadStar3Requirement", 0x31A03, 1, false},
    {"MipsStar1Requirement", 0xB34CB, 1, false},
    {"MipsStar2Requirement", 0xB3523, 1, false},

    {"ToadStar1Dialog", 0x31977, 1, true},
    {"ToadStar2Dialog", 0x3196B, 1, true},
    {"ToadStar3Dialog", 0x31983, 1, true},
    {"ToadStar1AfterDialog", 0x319BB, 1, true},
    {"ToadStar2AfterDialog", 0x319EF, 1, true},
    {"ToadStar3AfterDialog", 0x31A23, 1, true},
};
LevelTweak LevelTweakValues[] = {
   // {"entryLevel", 0x6D6B, 1},
   {"metalCapDuration", 0xAC0A, 2},
   {"wingCapDuration", 0xAC22, 2},
   {"vanishCapDuration", 0xABF2, 2},

   {"metalCapDurationCotmc", 0x4A5E, 2},
   {"wingCapDurationTotwc", 0x4A7A, 2},
   {"vanishCapDurationVcutm", 0x4A96, 2},
};

u32 ReadValueFromTweak(N64Rom &Rom, const ROMTweak &Tweak) {
    u32 Value = 0;
    if (Tweak.Size == 1) {
        Value = Rom.ReadBytesPhysical<u8>(Tweak.Address);
    } else if (Tweak.Size == 2) {
        Value = Rom.ReadBytesPhysical<u16>(Tweak.Address);
    } else if (Tweak.Size == 4) {
        Value = Rom.ReadBytesPhysical<u32>(Tweak.Address);
    }
    return Value;
}

std::string GetRomTweaks(N64Rom &Rom) {
    std::string Tweaks = "";

    for (const auto &StarPos : StarPositions) {
        f32 X, Y, Z;

        if (StarPos.isS16) {
            X = (f32)(Rom.ReadBytesPhysical<s16>(StarPos.Address));
            Y = (f32)(Rom.ReadBytesPhysical<s16>(StarPos.Address + 2));
            Z = (f32)(Rom.ReadBytesPhysical<s16>(StarPos.Address + 4));
        } else {
            X = Rom.ReadBytesPhysical<f32>(StarPos.Address);
            Y = Rom.ReadBytesPhysical<f32>(StarPos.Address + 4);
            Z = Rom.ReadBytesPhysical<f32>(StarPos.Address + 8);
        }

        Tweaks += "vec3f_set(gLevelValues.starPositions." + StarPos.Name + ", ";
        Tweaks += std::to_string((s32)X) + ", " + std::to_string((s32)Y) + ", " + std::to_string((s32)Z) + ")\n";
    }

    Tweaks += "\n";

    for (const auto &Tweak : BehaviorTweakValues){
        u32 Value = ReadValueFromTweak(Rom, Tweak);
        if (Tweak.Dialog) {
            Tweaks += "gBehaviorValues.dialogs." + Tweak.Name + " = " + std::to_string(Value) + "\n";
        } else {
            Tweaks += "gBehaviorValues." + Tweak.Name + " = " + std::to_string(Value) + "\n";
        }
    }

    Tweaks += "\n";

    for (const auto &Tweak : LevelTweakValues){
        u32 Value = ReadValueFromTweak(Rom, Tweak);
        Tweaks += "gLevelValues." + Tweak.Name + " = " + std::to_string(Value) + "\n";
    }

    return Tweaks;
}