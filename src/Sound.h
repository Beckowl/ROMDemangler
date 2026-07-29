#pragma once 

#include "LevelScript.h"

extern std::set<u8> SequenceMusics;

extern void ExportSequences(N64Rom &Rom);
extern u8 GetSeqNLST(N64Rom &Rom, u8 SeqID);