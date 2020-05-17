
#include "z80_test.h"

int testSingleADD(
    struct Z80State* z80, 
    void** memoryMap, 
    unsigned char* memory, 
    char* testOutput,
    int srcRegister,
    int baseInstruction,
    int cFlag
) {
    int run;
    unsigned char* srcByte;
    char instructionName[32];
    struct Z80State expected;
    int aValue;
    int srcValue;
    int aResult;
    int expectedRunLength;

    for (aValue = 0x7; aValue < 0x100; aValue = aValue + 0x08)
    {
        for (srcValue = 0x7; srcValue < 0x100; srcValue = srcValue + 0x08)
        { 
            if (srcRegister == A_REGISTER_INDEX)
            {
                srcValue = aValue;
            }

            srcByte = getRegisterPointer(z80, memory + 0x20, memory + 1, srcRegister);

            if (baseInstruction == Z80_ADD_A_B || baseInstruction == Z80_ADC_A_B || 
                baseInstruction == Z80_ADD_A_d8 || baseInstruction == Z80_ADC_A_d8)
            {
                aResult = aValue + srcValue;
            }
            else
            {
                aResult = aValue - srcValue;
            }

            if ((baseInstruction == Z80_ADC_A_B || baseInstruction == Z80_ADC_A_d8) && cFlag)
            {
                ++aResult;
            }
            else if ((baseInstruction == Z80_SBC_A_B || (baseInstruction == Z80_SBC_A_d8)) && cFlag)
            {
                --aResult;
            }

            initializeZ80(z80);
            z80->a = aValue;
            z80->h = 0x80;
            z80->l = 0x20;
            *srcByte = srcValue;

            if (cFlag)
            {
                z80->f = GB_FLAGS_C;
            }

            expected = *z80;
            expected.pc = (srcRegister == d8_REGISTER_INDEX) ? 2 : 1;
            if (baseInstruction != Z80_CP_A_B && baseInstruction != Z80_CP_A_d8)
            {
                expected.a = 0xFF & aResult;
            }
            expected.f = 0;
            expectedRunLength = (srcRegister == HL_REGISTER_INDEX || srcRegister == d8_REGISTER_INDEX) ? 2 : 1;

            if (aResult & 0x100)
            {
                expected.f |= GB_FLAGS_C;
            }

            if (!(aResult & 0xFF))
            {
                expected.f |= GB_FLAGS_Z;
            }

            if ((aResult ^ aValue ^ srcValue) & 0x10)
            {
                expected.f |= GB_FLAGS_H;
            }

            if (baseInstruction == Z80_SUB_A_B || baseInstruction == Z80_SBC_A_B || baseInstruction == Z80_CP_A_B ||
                baseInstruction == Z80_SUB_A_d8 || baseInstruction == Z80_SBC_A_d8 || baseInstruction == Z80_CP_A_d8)
            {
                expected.f |= GB_FLAGS_N;
            }

            if (baseInstruction < 0xC0)
            {            
                memory[0] = baseInstruction + srcRegister;
            }
            else
            {
                memory[0] = baseInstruction;
            }

            run = runZ80CPU(z80, memoryMap, 1);

            if (baseInstruction == Z80_ADD_A_B || baseInstruction == Z80_ADD_A_d8) 
            {
                sprintf(instructionName, "ADD A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else if (baseInstruction == Z80_ADC_A_B || baseInstruction == Z80_ADC_A_d8) 
            {
                sprintf(instructionName, "ADC A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else if (baseInstruction == Z80_SUB_A_B || baseInstruction == Z80_SUB_A_d8) 
            {
                sprintf(instructionName, "SUB A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else if (baseInstruction == Z80_SBC_A_B || baseInstruction == Z80_SBC_A_d8) 
            {
                sprintf(instructionName, "SBC A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else if (baseInstruction == Z80_CP_A_B || baseInstruction == Z80_CP_A_d8) 
            {
                sprintf(instructionName, "CP A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else 
            {
                sprintf(testOutput, "Invalid instruction base %X", baseInstruction);
                return 0;
            }

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

int testADD_ADC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int srcRegister;

    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {       
        if (
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_ADD_A_B, 0) ||
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_ADD_A_B, 1) ||
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_ADC_A_B, 0) ||
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_ADC_A_B, 1) ||
            0
        )
        {
            return 0;
        }
    }

    return 1;
}

int testSUB_SBC(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int srcRegister;

    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {       
        if (
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_SUB_A_B, 0) ||
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_SUB_A_B, 1) ||
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_SBC_A_B, 0) ||
            !testSingleADD( z80, memoryMap, memory, testOutput, srcRegister, Z80_SBC_A_B, 1) ||
            0
        )
        {
            return 0;
        }
    }

    return 1;
}

int run0x8_9Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testADD_ADC(z80, memoryMap, memory, testOutput) &&
        testSUB_SBC(z80, memoryMap, memory, testOutput) &&
        1
    ;
}