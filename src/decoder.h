
#ifndef _DECODER_H
#define _DECODER_H

#include <ultra64.h>
#include "bool.h"

#define INST_FLAGS_BRANCH              0x01
#define INST_FLAGS_ADDRESS             0x02
#define INST_FLAGS_RELATIVE_ADDRESS    0x04
#define INST_FLAGS_RETURN              0x08

struct InstructionInformation {
    char* formatString;
    u8 extraBytes;
    u8 flags;
};

struct AddressTuple {
    u16 nextInstruction;
    u16 branchInstruction;
};

int decodeInstruction(char* output, u8* input, u16 address);
int getInstructionSize(u8 instructionValue);
u8* scanForInstruction(u8* input, u8* upperBound, u8 instruction);
struct AddressTuple getInstructionBranch(u8* instruction, u16 address, u16 topOfStack, u16 hl);
bool isInstructionCall(u8 instrction);

#endif