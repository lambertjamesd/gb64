
#ifndef _DECODER_H
#define _DECODER_H

#include <ultra64.h>

struct InstructionInformation {
    char* formatString;
    u8 extraBytes;
    u8 relativeAddress; 
};

int decodeInstruction(char* output, u8* input, u16 address);
int getInstructionSize(u8 instructionValue);
u8* scanForInstruction(u8* input, u8* upperBound, u8 instruction);

#endif