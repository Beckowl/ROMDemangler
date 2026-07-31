#pragma once 

#include "LevelScript.h"

extern std::set<u8> SequenceMusics;
extern std::vector<std::string> SequenceNames;

extern void ExportSequences(N64Rom &Rom);
extern void GetSequenceNames(N64Rom &Rom);
extern u8 GetSeqNInst(N64Rom &Rom, u8 SeqID);