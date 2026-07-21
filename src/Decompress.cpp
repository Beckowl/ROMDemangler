#include "Decompress.h"

std::vector<u8> DecompressMIO0(N64Rom &Rom, u32 RomStart) {
    auto ReadU32 = [&](u32 Off) { return Rom.ReadBytesPhysical<u32>(Off); };
    auto ReadU16 = [&](u32 Off) { return Rom.ReadBytesPhysical<u16>(Off); };
    auto ReadU8  = [&](u32 Off) { return Rom.ReadBytesPhysical<u8>(Off); };
    
    u32 OutSize = ReadU32(RomStart + 0x04);
    u32 CompOff = ReadU32(RomStart + 0x08);
    u32 RawOff  = ReadU32(RomStart + 0x0C);
    
    u32 CtrlPos = RomStart + 0x10;
    u32 CompPos = RomStart + CompOff;
    u32 RawPos  = RomStart + RawOff;
    
    std::vector<u8> Out(OutSize);
    u32 OutPos = 0;
    
    u8 Mask = 0;
    u8 Ctrl = 0;
    
    while (OutPos < OutSize) {
        if (Mask == 0) {
            Ctrl = ReadU8(CtrlPos++);
            Mask = 0x80;
        }
        
        if (Ctrl & Mask) {
            Out[OutPos++] = ReadU8(RawPos++);
        } else {
            u16 val = ReadU16(CompPos);
            CompPos += 2;
            
            u32 Length = (val >> 12) + 3;
            u32 Offset = (val & 0x0FFF) + 1;
            
            for (u32 i = 0; i < Length; i++) {
                Out[OutPos] = Out[OutPos - Offset];
                OutPos++;
            }
        }
        
        Mask >>= 1;
    }
    
    return Out;
}

std::vector<u8> DecompressYAY0(N64Rom &Rom, u32 RomStart) {
    auto ReadU32 = [&](u32 Off) { return Rom.ReadBytesPhysical<u32>(Off); };
    auto ReadU16 = [&](u32 Off) { return Rom.ReadBytesPhysical<u16>(Off); };
    auto ReadU8  = [&](u32 Off) { return Rom.ReadBytesPhysical<u8>(Off); };
    
    u32 OutSize = ReadU32(RomStart + 0x04);
    u32 LinkOff = ReadU32(RomStart + 0x08);
    u32 NonLinkOff = ReadU32(RomStart + 0x0C);
    
    u32 CtrlPos = RomStart + 0x10;
    u32 LinkPos = RomStart + LinkOff;
    u32 NonLinkPos = RomStart + NonLinkOff;
    
    std::vector<u8> Out(OutSize);
    u32 OutPos = 0;
    
    u32 Mask = 0;
    u32 BitsLeft = 0;
    
    while (OutPos < OutSize) {
        if (BitsLeft == 0) {
            Mask = ReadU32(CtrlPos);
            CtrlPos += 4;
            BitsLeft = 32;
        }
        
        if (Mask & 0x80000000) {
            Out[OutPos++] = ReadU8(NonLinkPos++);
        } else {
            u16 LinkVal = ReadU16(LinkPos);
            LinkPos += 2;
            
            u32 Length = LinkVal >> 12;
            u32 Offset = (LinkVal & 0x0FFF) + 1;
            
            if (Length == 0) {
                Length = ReadU8(NonLinkPos++) + 0x12;
            } else {
                Length += 2;
            }
            
            for (u32 i = 0; i < Length; i++) {
                Out[OutPos] = Out[OutPos - Offset];
                OutPos++;
            }
        }
        
        Mask <<= 1;
        BitsLeft--;
    }
    
    return Out;
}


struct HuffmanEntry {
    u16 CodeValue = 0;
    u8 BitDepth = 0;
};

struct RNCDecoderState {
    N64Rom* Rom = nullptr;
    u32 ReadOffset = 0;
    
    std::vector<u8> OutBuffer;
    u32 WriteOffset = 0;
    u32 TargetSize = 0;

    s32 BitCount = 0;
    u32 BitStreamM1 = 0;
    u8 BitStreamM2 = 0;

    HuffmanEntry RawTable[16];
    HuffmanEntry DistTable[16];
    HuffmanEntry LenTable[16];
};

static u32 FlipBits(u32 Bits, u32 Amount) {
    u32 Flipped = 0;
    for (u32 I = 0; I < Amount; I++) {
        Flipped = (Flipped << 1) | (Bits & 1);
        Bits >>= 1;
    }
    return Flipped;
}

static void BuildHuffmanTree(HuffmanEntry* Table, u32 Count) {
    u32 CurrentDepth = 1;
    u32 CodeAccumulator = 0;
    u32 BaseStep = 0x80000000;

    while (CurrentDepth <= 16) {
        for (u32 I = 0; I < Count; I++) {
            if (Table[I].BitDepth == CurrentDepth) {
                Table[I].CodeValue = FlipBits(CodeAccumulator / BaseStep, CurrentDepth);
                CodeAccumulator += BaseStep;
            }
        }
        CurrentDepth++;
        BaseStep >>= 1;
    }
}

static u32 FetchLe32(RNCDecoderState &State) {
    u32 Result = 0;
    for (u32 I = 0; I < 4; I++) {
        Result |= (State.Rom->ReadBytesPhysical<u8>(State.ReadOffset + I) << (I * 8));
    }
    return Result;
}

static u32 ExtractBitsM1(RNCDecoderState &State, s32 Count) {
    u32 Extracted = 0;
    u32 Mask = 1;
    while (Count > 0) {
        if (State.BitCount == 0) {
            State.BitStreamM1 = FetchLe32(State);
            State.ReadOffset += 2;
            State.BitCount = 16;
        }

        if (State.BitStreamM1 & 1) {
            Extracted |= Mask;
        }
        Mask <<= 1;
        State.BitStreamM1 >>= 1;
        State.BitCount -= 1;
        Count -= 1;
    }
    return Extracted;
}

static u32 ExtractBitsM2(RNCDecoderState &State, s32 Count) {
    u32 Extracted = 0;
    while (Count > 0) {
        if (State.BitCount == 0) {
            State.BitStreamM2 = State.Rom->ReadBytesPhysical<u8>(State.ReadOffset++);
            State.BitCount = 8;
        }

        Extracted <<= 1;
        if (State.BitStreamM2 & 0x80) {
            Extracted |= 1;
        }
        State.BitStreamM2 = (State.BitStreamM2 << 1) & 0xFF;
        State.BitCount -= 1;
        Count -= 1;
    }
    return Extracted;
}

static void LoadHuffmanTable(RNCDecoderState &State, HuffmanEntry* Table) {
    for (u32 I = 0; I < 16; I++) {
        Table[I].BitDepth = 0;
        Table[I].CodeValue = 0;
    }

    u32 EntryCount = ExtractBitsM1(State, 5);
    if (EntryCount > 16) EntryCount = 16;
    if (EntryCount == 0) return;

    for (u32 I = 0; I < EntryCount; I++) {
        Table[I].BitDepth = (u8)ExtractBitsM1(State, 4);
    }

    BuildHuffmanTree(Table, EntryCount);
}

static u32 ReadHuffmanValue(RNCDecoderState &State, HuffmanEntry* Table) {
    u32 Index = 0;
    while (Index < 16 && (Table[Index].BitDepth == 0 || 
           (State.BitStreamM1 & ((1 << Table[Index].BitDepth) - 1)) != Table[Index].CodeValue)) {
        Index++;
    }

    if (Index >= 16) {
        printf("RNC Unpack Error: Invalid Huffman code\n");
        return 0;
    }

    ExtractBitsM1(State, Table[Index].BitDepth);
    if (Index < 2) return Index;
    return ExtractBitsM1(State, Index - 1) | (1 << (Index - 1));
}

static u32 DecodeLengthM2(RNCDecoderState &State) {
    u32 Length = ExtractBitsM2(State, 1) + 4;
    if (ExtractBitsM2(State, 1) == 0) return Length;
    return ((Length - 1) << 1) + ExtractBitsM2(State, 1);
}

static u32 DecodePositionM2(RNCDecoderState &State) {
    u32 Position = 0;
    if (ExtractBitsM2(State, 1)) {
        Position = ExtractBitsM2(State, 1);
        if (ExtractBitsM2(State, 1)) {
            Position = ((Position << 1) + ExtractBitsM2(State, 1)) | 4;
            if (ExtractBitsM2(State, 1) == 0) {
                Position = (Position << 1) + ExtractBitsM2(State, 1);
            }
        } else if (Position == 0) {
            Position = ExtractBitsM2(State, 1) + 2;
        }
    }

    u32 LowByte = State.Rom->ReadBytesPhysical<u8>(State.ReadOffset++);
    return (Position << 8) + LowByte + 1;
}

static void ProcessMethod1(RNCDecoderState &State) {
    ExtractBitsM1(State, 2);
    while (State.WriteOffset < State.TargetSize) {
        LoadHuffmanTable(State, State.RawTable);
        LoadHuffmanTable(State, State.DistTable);
        LoadHuffmanTable(State, State.LenTable);
        
        u32 ChunkCount = ExtractBitsM1(State, 16);
        bool IsInitialChunk = true;

        while (ChunkCount > 0) {
            if (!IsInitialChunk) {
                u32 Distance = ReadHuffmanValue(State, State.DistTable) + 1;
                u32 MatchLen = ReadHuffmanValue(State, State.LenTable) + 2;
                
                for (u32 I = 0; I < MatchLen; I++) {
                    if (State.WriteOffset < State.TargetSize) {
                        State.OutBuffer[State.WriteOffset] = State.OutBuffer[State.WriteOffset - Distance];
                        State.WriteOffset++;
                    }
                }
            }

            u32 RawLen = ReadHuffmanValue(State, State.RawTable);
            for (u32 I = 0; I < RawLen; I++) {
                if (State.WriteOffset < State.TargetSize) {
                    State.OutBuffer[State.WriteOffset++] = State.Rom->ReadBytesPhysical<u8>(State.ReadOffset++);
                }
            }

            u32 NextWord = FetchLe32(State);
            u32 BitMask = (1 << State.BitCount) - 1;
            State.BitStreamM1 = ((NextWord << State.BitCount) | (State.BitStreamM1 & BitMask)) & 0xFFFFFFFF;

            IsInitialChunk = false;
            ChunkCount -= 1;
        }
    }
}

static void ProcessMethod2(RNCDecoderState &State) {
    ExtractBitsM2(State, 2);
    while (State.WriteOffset < State.TargetSize) {
        while (true) {
            while (ExtractBitsM2(State, 1) == 0 && State.WriteOffset < State.TargetSize) {
                State.OutBuffer[State.WriteOffset++] = State.Rom->ReadBytesPhysical<u8>(State.ReadOffset++);
            }

            if (ExtractBitsM2(State, 1)) {
                u32 MatchLen;
                u32 Distance;
                if (ExtractBitsM2(State, 1) == 0) {
                    MatchLen = 2;
                    Distance = State.Rom->ReadBytesPhysical<u8>(State.ReadOffset++) + 1;
                } else {
                    if (ExtractBitsM2(State, 1) == 0) {
                        MatchLen = 3;
                    } else {
                        MatchLen = State.Rom->ReadBytesPhysical<u8>(State.ReadOffset++) + 8;
                        if (MatchLen == 8) break;
                    }
                    Distance = DecodePositionM2(State);
                }

                for (u32 I = 0; I < MatchLen; I++) {
                    if (State.WriteOffset < State.TargetSize) {
                        State.OutBuffer[State.WriteOffset] = State.OutBuffer[State.WriteOffset - Distance];
                        State.WriteOffset++;
                    }
                }
            } else {
                u32 MatchLen = DecodeLengthM2(State);
                if (MatchLen == 9) {
                    MatchLen = (ExtractBitsM2(State, 4) << 2) + 12;
                    for (u32 I = 0; I < MatchLen; I++) {
                        if (State.WriteOffset < State.TargetSize) {
                            State.OutBuffer[State.WriteOffset++] = State.Rom->ReadBytesPhysical<u8>(State.ReadOffset++);
                        }
                    }
                } else {
                    u32 Distance = DecodePositionM2(State);
                    for (u32 I = 0; I < MatchLen; I++) {
                        if (State.WriteOffset < State.TargetSize) {
                            State.OutBuffer[State.WriteOffset] = State.OutBuffer[State.WriteOffset - Distance];
                            State.WriteOffset++;
                        }
                    }
                }
            }
        }
        ExtractBitsM2(State, 1);
    }
}

std::vector<u8> DecompressRNC(N64Rom &Rom, u32 RomStart) {
    u8 CompressionMethod = Rom.ReadBytesPhysical<u8>(RomStart + 3);
    
    u32 TargetSize = 0;
    for (int I = 0; I < 4; I++) {
        TargetSize = (TargetSize << 8) | Rom.ReadBytesPhysical<u8>(RomStart + 4 + I);
    }

    if (CompressionMethod == 0) {
        std::vector<u8> UncompressedData(TargetSize);
        for (u32 I = 0; I < TargetSize; I++) {
            UncompressedData[I] = Rom.ReadBytesPhysical<u8>(RomStart + 0x12 + I);
        }
        return UncompressedData;
    }

    RNCDecoderState State;
    State.Rom = &Rom;
    State.ReadOffset = RomStart + 0x12;
    State.OutBuffer.resize(TargetSize);
    State.WriteOffset = 0;
    State.TargetSize = TargetSize;
    State.BitCount = 0;
    State.BitStreamM1 = 0;
    State.BitStreamM2 = 0;

    if (CompressionMethod == 1) {
        ProcessMethod1(State);
    } else if (CompressionMethod == 2) {
        ProcessMethod2(State);
    } else {
        printf("Unsupported RNC Method: %d\n", CompressionMethod);
        return {};
    }

    return State.OutBuffer;
}