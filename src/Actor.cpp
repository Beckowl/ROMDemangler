#include "Actor.h"
#include "LevelScript.h"
#include "GeoLayout.h"
#include "Model.h"
#include "Memory.h"


std::map<u32, bool> ExportedActors;

void ExtractGeoDisplayLists(N64Rom &Rom, u32 SegAddr, std::vector<u32> &OutDLs, std::map<u32, bool> &SeenGeos) {
    if (!SegAddr || SeenGeos[SegAddr]) return;
    SeenGeos[SegAddr] = true;

    auto PushDL = [&](u32 DL) {
        if (ValidateMemAddr(DL)) OutDLs.push_back(DL);
        else if (DL) printf("DisplayList 0x%08x has an invalid address, ignoring export\n", DL);
    };

    u32 Entry = SegAddr;
    while (true) {
        u8 Cmd = Rom.ReadBytes<u8>(Entry);
        u8 Len = GetGeolayoutCmdSize(Rom, Entry);

        if (Cmd == 0x01 || Entry == UINT32_MAX) break;

        if (Cmd == 0x02) {
            u32 NewSegAddr = Rom.ReadBytes<u32>(Entry + 4);
            ExtractGeoDisplayLists(Rom, NewSegAddr, OutDLs, SeenGeos);
            if (Rom.ReadBytes<u8>(Entry + 1) == 0) break;
        } else if (Cmd == 0x11) {
            if (Rom.ReadBytes<u8>(Entry + 1) & 0x80) {
                u32 DL = Rom.ReadBytes<u32>(Entry + 8);
                PushDL(DL);
            }
        } else if (Cmd == 0x13) {
            u32 DL = Rom.ReadBytes<u32>(Entry + 8);
            PushDL(DL);
        } else if (Cmd == 0x15) {
            u32 DL = Rom.ReadBytes<u32>(Entry + 4);
            PushDL(DL);
        } else if (Cmd == 0x1D) {
            if (Rom.ReadBytes<u8>(Entry + 1) & 0x80) {
                u32 DL = Rom.ReadBytes<u32>(Entry + 8);
                PushDL(DL);
            }
        } else if (Cmd == 0x10) {
            u8 Layer = Rom.ReadBytes<u8>(Entry + 1);
            if (Layer & 0x30) {
                if (Layer & 0x80) {
                    PushDL(Rom.ReadBytes<u32>(Entry + 4));
                }
            } else if (Layer & 0x20 || Layer & 0x10) {
                if (Layer & 0x80) {
                    PushDL(Rom.ReadBytes<u32>(Entry + 8));
                }
            } else {
                if (Layer & 0x80) {
                    PushDL(Rom.ReadBytes<u32>(Entry + 16));
                }
            }
        }

        Entry += Len;
    }
}

void ExportActors(N64Rom &Rom, LevelScript &Script) {
    std::string ActorsPath = "output/actors";
    fs::create_directories(ActorsPath);

    for (auto &Act : Script.Actors) {
        if (!ValidateMemAddr(Act.Addr)) {
            printf("Actor 0x%08x has an invalid address, ignoring export\n", Act.Addr);
            continue;
        }
        if (ExportedActors[Act.Addr] && (Act.Addr >> 24) == 0) continue;
        
        std::string ActorFolder = (ActorsPath + "/" + Act.Name);
        fs::create_directories(ActorFolder);

        Script.CurrentActor = &Act;

        if (!Act.IsDL) {
            std::map<u32, bool> SeenGeos;
            ExtractGeoDisplayLists(Rom, Act.Addr, Act.DisplayLists, SeenGeos);
            
            std::string GeoDumpPath = ActorFolder + "/geo.inc.c";
            printf("Exporting actor %s\n", Act.Name.c_str());
            ExportGeolayout(Rom, 0, Act.Name, Act.Addr, Act.Addr, Script, GeoDumpPath.c_str());
        } else {
            Act.DisplayLists.push_back(Act.Addr);
        }

        std::string ModelDumpPath = ActorFolder + "/model.inc.c";
        ExportModels(Rom, Script, "", 0, ModelDumpPath.c_str(), true, &Act);

        Script.CurrentActor = nullptr;
        ExportedActors[Act.Addr] = true;
    }
}