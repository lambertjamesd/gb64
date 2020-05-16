
#include "z80_test.h"

int testSingleBitwise(
    struct Z80State* z80, 
    void** memoryMap, 
    unsigned char* memory, 
    char* testOutput,
    int srcRegister,
    int baseInstruction
)
{
    int run;
    unsigned char* srcByte;
    char instructionName[32];
    struct Z80State expected;
    int aValue;
    int srcValue;
    int expectedRunLength;

    for (aValue = 0x7; aValue < 0x100; aValue = aValue + 0x08)
    {
        for (srcValue = 0x7; srcValue < 0x100; srcValue = srcValue + 0x08)
        { 
            if (srcRegister == A_REGISTER_INDEX)
            {
                srcValue = aValue;
            }

            srcByte = getRegisterPointer(z80, memory + 0x20, srcRegister);

            initializeZ80(z80);
            z80->a = aValue;
            z80->h = 0x80;
            z80->l = 0x20;
            *srcByte = srcValue;
            expected = *z80;
            expected.pc = 1;
            expectedRunLength = (srcRegister == HL_REGISTER_INDEX) ? 2 : 1;

            memory[0] = baseInstruction + srcRegister;
            
            if (baseInstruction == Z80_AND_A_B)
            {
                expected.a = aValue & srcValue;
                expected.f = GB_FLAGS_H;
                sprintf(instructionName, "AND A (%X) %s (%X)", aValue, registerNames[srcRegister], srcValue);
            } 
            else if (baseInstruction == Z80_XOR_A_B)
            {
                expected.a = aValue ^ srcValue;
                sprintf(instructionName, "XOR A (%X) %s (%X)", aValue, registerNames[srcRegister], srcValue);
            } 
            else if (baseInstruction == Z80_OR_A_B) 
            {
                expected.a = aValue | srcValue;
                sprintf(instructionName, "OR A (%X) %s (%X)", aValue, registerNames[srcRegister], srcValue);
            } 
            else 
            {
                sprintf(testOutput, "Invalid instruction base %X", baseInstruction);
                return 0;
            }

            if (expected.a == 0)
            {
                expected.f |= GB_FLAGS_Z;
            }

            run = runZ80CPU(z80, memoryMap, 1);

            if (
                !testZ80State(instructionName, testOutput, z80, &expected) ||
                !testInt(instructionName, testOutput, run, expectedRunLength)
            )
            {
                return 0;
            }

            if (srcRegister == A_REGISTER_INDEX)
            {
                break;
            }
        }
    }

    return 1;
}

int testAND_XOR_OR(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int srcRegister;

    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {       
        if (
            !testSingleBitwise( z80, memoryMap, memory, testOutput, srcRegister, Z80_AND_A_B) ||
            !testSingleBitwise( z80, memoryMap, memory, testOutput, srcRegister, Z80_XOR_A_B) ||
            !testSingleBitwise( z80, memoryMap, memory, testOutput, srcRegister, Z80_OR_A_B) ||
            0
        )
        {
            return 0;
        }
    }

    return 1;
}

int testCP(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int srcRegister;

    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {       
        if (
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_CP_A_B, 0) ||
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_CP_A_B, 1) ||
            0
        )
        {
            return 0;
        }
    }

    return 1;
}

int run0xA_BTests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testCP(z80, memoryMap, memory, testOutput) &&
        testAND_XOR_OR(z80, memoryMap, memory, testOutput) &&
        1
    ;
}