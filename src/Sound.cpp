#include "Sound.h"

std::vector<u8> SequenceMusics;

void ExportSequence(N64Rom &Rom, u8 SeqID, const char *FilePath) {
    FILE *SeqDump = fopen(FilePath, "wb");

    u32 SeqFileHeader = (Rom.ReadBytesPhysical<u32>(0xD4768) & 0xFFFF) << 16;
    SeqFileHeader += (Rom.ReadBytesPhysical<u32>(0xD4770) & 0xFFFF);
    u32 SeqFileOffset = SeqFileHeader + SeqID * 8 + 4;
    u32 SeqOffset = Rom.ReadBytesPhysical<u32>(SeqFileOffset);
    u32 SeqLength = Rom.ReadBytesPhysical<u32>(SeqFileOffset + 4);
    std::vector<u8> M64Data(SeqLength);
    Rom.ReadBytesPtr<u8>(SeqFileHeader + SeqOffset, M64Data.data(), SeqLength);
    fwrite(M64Data.data(), 1, SeqLength, SeqDump);

    fclose(SeqDump);
}

void ExportSequences(N64Rom &Rom) {
    std::string MusicPath = "output/sound";
    fs::create_directories(MusicPath);

    printf("Exporting sequences\n");

    for (auto &I : SequenceMusics) {
        char FileName[256];
        sprintf(FileName, "seq_%02d.m64", I);
        std::string FilePath = MusicPath + "/" + std::string(FileName);

        ExportSequence(Rom, I, FilePath.c_str());
    }
}

u8 GetSeqBank(N64Rom &Rom, u8 SeqID) {
    u32 SeqMagic = (GameType == GT_DECOMP) ? 0x7b0800 : 0x7f0000;

    u32 Entry = SeqMagic + SeqID * 2;

    u16 BankOff = Rom.ReadBytesPhysical<u16>(Entry);
    u32 BankByteOff = SeqMagic + (u32)(BankOff) + 1;
    u8 Bank = Rom.ReadBytesPhysical<u8>(BankByteOff);

    return Bank;
}