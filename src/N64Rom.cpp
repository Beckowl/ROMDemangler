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
    mFile = fopen(Path, "rb");
    if (!mFile) {
        printf("Can't open \"%s\"\n", Path);
        exit(1);
    }
    fseek(mFile, 0, SEEK_END);
    mSize = ftell(mFile);
    fseek(mFile, 0, SEEK_SET);
    mData = (u8 *) malloc(mSize);
    fread(mData, 1, mSize, mFile);
    mRomInternalName.resize(0x15);
    ReadBytesPtr<char>(0x20, &mRomInternalName[0], 0x15);

    if (RAMPath) {
        FILE *RamFILE = fopen(RAMPath, "rb");
        if (!RamFILE) {
            printf("Can't open \"%s\"\n", RAMPath);
            exit(1);
        }
        fseek(RamFILE, 0, SEEK_END);
        size_t RAMSize = ftell(RamFILE);
        fseek(RamFILE, 0, SEEK_SET);
        RAM = (u8 *) malloc(RAMSize);
        fread(RAM, 1, RAMSize, RamFILE);
        fclose(RamFILE);
    }

    mMicrocode = UCODE_UNKNOWN;
    std::string FoundUCode = "";
    std::string ExtraConfig = "";

    char *SigPtr = (char*)memmem(mData, mSize, "RSP Gfx ucode ", 14);
    if (!SigPtr) {
        // old f3d
        SigPtr = (char*)memmem(mData, mSize, "RSP SW Version: ", 16);
    }

    if (SigPtr) {
        size_t SigLen = 0;
        while (SigLen < 80 && SigPtr[SigLen] != '\0' && SigPtr[SigLen] != '\n' && SigPtr[SigLen] != '\r') {
            SigLen++;
        }
        FoundUCode = std::string(SigPtr, SigLen);

        if (FoundUCode.find("F3DZEX") != std::string::npos) {
            mMicrocode = UCODE_F3DZEX;
        } else if (FoundUCode.find("F3DEX") != std::string::npos) {
            if (FoundUCode.find("2.") != std::string::npos) {
                mMicrocode = UCODE_F3DEX2;
            } else {
                mMicrocode = UCODE_F3DEX;
            }
        } else if (FoundUCode.find("RSP SW Version: 2.0") != std::string::npos) {
            mMicrocode = UCODE_F3D;
        }

        if (FoundUCode.find("fifo") != std::string::npos) ExtraConfig += " [FIFO]";
        if (FoundUCode.find("xbus") != std::string::npos) ExtraConfig += " [XBUS]";
        if (FoundUCode.find("NoN") != std::string::npos) ExtraConfig += " [NoN]";
    }

    if (mMicrocode == UCODE_UNKNOWN) {
        printf("Unsupported Microcode detected!\n");
        exit(1);
    }

    printf("Microcode Details:\n");
    printf("  Signature: %s\n", FoundUCode.c_str());
    printf("  Name: %s\n", UCodeSigToName[mMicrocode].c_str());
    if (!ExtraConfig.empty()) {
        printf("  Features:%s\n", ExtraConfig.c_str());
    }

    // its just ex2 with positional lighting that
    // no one ever used lmfao
    if (mMicrocode == UCODE_F3DZEX) {
        mMicrocode = UCODE_F3DEX2; 
    }
}

void N64Rom::CloseFile(void) {
    free(mData);
    if (RAM) free(RAM);
    fclose(mFile);
}