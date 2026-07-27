#include "N64Rom.h"
#include <map>

std::map<enum N64Microcode, std::string> UCodeSigToName = {
    { UCODE_F3D, "F3D" },
    { UCODE_F3DEX, "F3DEX" },
    { UCODE_F3DEX2, "F3DEX2" },
    { UCODE_F3DZEX, "F3DZEX" },
};

#ifdef _WIN32
void *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen) {
    if (needlelen == 0)
        return (void *)haystack;

    if (needlelen > haystacklen)
        return NULL;

    const unsigned char *h = (const unsigned char *)haystack;
    const unsigned char *n = (const unsigned char *)needle;
    const unsigned char *end = h + (haystacklen - needlelen);

    for (const unsigned char *p = h; p <= end; ++p) {
        if (*p == *n) {
            if (memcmp(p + 1, n + 1, needlelen - 1) == 0)
                return (void *)p;
        }
    }
    return NULL;
}
#endif

void N64Rom::OpenFile(const char *Path, const char *RAMPath) {
    File = fopen(Path, "rb");
    if (!File) {
        printf("Can't open \"%s\"\n", Path);
        exit(1);
    }
    fseek(File, 0, SEEK_END);
    Size = ftell(File);
    fseek(File, 0, SEEK_SET);
    Data = new u8[Size];
    fread(Data, 1, Size, File);
    ROMInternalName.resize(0x15);
    ReadBytesPtr<char>(0x20, &ROMInternalName[0], 0x15);

    if (RAMPath) {
        FILE *RAMFile = fopen(RAMPath, "rb");
        if (!RAMFile) {
            printf("Can't open \"%s\"\n", RAMPath);
            exit(1);
        }
        fseek(RAMFile, 0, SEEK_END);
        size_t RAMSize = ftell(RAMFile);
        fseek(RAMFile, 0, SEEK_SET);
        RAM = new u8[RAMSize];
        fread(RAM, 1, RAMSize, RAMFile);
        fclose(RAMFile);
    }

    Microcode = UCODE_UNKNOWN;
    std::string FoundUCode = "";
    std::string ExtraConfig = "";

    char *SigPtr = (char*)memmem(Data, Size, "RSP Gfx ucode ", 14);
    if (!SigPtr) {
        // old f3d
        SigPtr = (char*)memmem(Data, Size, "RSP SW Version: ", 16);
    }

    if (SigPtr) {
        size_t SigLen = 0;
        while (SigLen < 80 && SigPtr[SigLen] != '\0' && SigPtr[SigLen] != '\n' && SigPtr[SigLen] != '\r') {
            SigLen++;
        }
        FoundUCode = std::string(SigPtr, SigLen);

        if (FoundUCode.find("F3DZEX") != std::string::npos) {
            Microcode = UCODE_F3DZEX;
        } else if (FoundUCode.find("F3DEX") != std::string::npos) {
            if (FoundUCode.find("2.") != std::string::npos) {
                Microcode = UCODE_F3DEX2;
            } else {
                Microcode = UCODE_F3DEX;
            }
        } else if (FoundUCode.find("RSP SW Version: 2.0") != std::string::npos) {
            Microcode = UCODE_F3D;
        }

        if (FoundUCode.find("fifo") != std::string::npos) ExtraConfig += " [FIFO]";
        if (FoundUCode.find("xbus") != std::string::npos) ExtraConfig += " [XBUS]";
        if (FoundUCode.find("NoN") != std::string::npos) ExtraConfig += " [NoN]";
    }

    if (Microcode == UCODE_UNKNOWN) {
        printf("Unsupported Microcode detected!\n");
        exit(1);
    }

    printf("Microcode Details:\n");
    printf("  Signature: %s\n", FoundUCode.c_str());
    printf("  Name: %s\n", UCodeSigToName[Microcode].c_str());
    if (!ExtraConfig.empty()) {
        printf("  Features:%s\n", ExtraConfig.c_str());
    }

    // its just ex2 with positional lighting that
    // no one ever used lmfao
    if (Microcode == UCODE_F3DZEX) {
        Microcode = UCODE_F3DEX2; 
    }
}

void N64Rom::CloseFile(void) {
    delete[] Data;
    if (RAM) delete[] RAM;
    fclose(File);
}