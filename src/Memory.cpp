#include "Memory.h"
#include "Main.h"
#include <format>

std::vector<u8> SegmentData[MAX_SEGMENT];
u32 SegmentOffsets[MAX_SEGMENT][2] = {0};
FILE *MapFile = fopen("sm64.us.map", "r");

std::string GetLabelFromMap(u32 Address) {
    rewind(MapFile);

    char Line[1024];
    while (fgets(Line, sizeof(Line), MapFile)) {
        size_t n = strlen(Line);
        while (n > 0 && (Line[n-1] == '\n' || Line[n-1] == '\r' || isspace((unsigned char)Line[n-1]))) {
            Line[--n] = '\0';
        }

        char Tmp[1024];
        strncpy(Tmp, Line, sizeof(Tmp));
        Tmp[sizeof(Tmp)-1] = '\0';

        char *Tokens[64];
        int TCount = 0;
        char *Token = strtok(Tmp, " \t");
        while (Token && TCount < (int)(sizeof(Tokens)/sizeof(Tokens[0]))) {
            Tokens[TCount++] = Token;
            Token = strtok(NULL, " \t");
        }

        if (TCount == 0) continue;

        for (int i = 0; i < TCount; ++i) {
            char *End = NULL;
            unsigned long Val = strtoul(Tokens[i], &End, 16);
            if (End == Tokens[i]) continue;
            if (*End != '\0') continue;

            if ((u32)Val == Address) {
                char *Label = Tokens[TCount - 1];
                return std::string(Label);
            }
        }
    }

    std::string Buf = std::format(
        "Custom_{:#x}", 
        Address
    );
    return Buf;
}

bool ValidateMemAddr(u32 Address) {
    s32 Bank = Address >> 24;
    if (Bank > (MAX_SEGMENT - 1)) {
        return false;
    }
    if (!Address) {
        return false;
    }
    if (IgnoreSegment0 && Bank == 0) {
        return false;
    }
    return true;
}
