#pragma once 

#include "LevelScript.h"

extern std::vector<u8> SequenceMusics;

extern void ExportSequences(N64Rom &Rom);
extern u8 GetSeqBank(N64Rom &Rom, u8 SeqID);