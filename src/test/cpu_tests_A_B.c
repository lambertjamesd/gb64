
#include "cpu_test.h"

int testSingleBitwise(
    struct CPUState* cpu, 
    struct Memory* memory,
    char* testOutput,
    int srcRegister,
    int baseInstruction
)
{
    int run;
    unsigned char* srcByte;
    char instructionName[32];
    struct CPUState expected;
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

            srcByte = getRegisterPointer(cpu, memory->internalRam + 0x20, memory->internalRam + 1, srcRegister);

            initializeCPU(cpu);
            cpu->a = aValue;
            cpu->h = 0x80;
            cpu->l = 0x20;
            *srcByte = srcValue;
            expected = *cpu;
            expected.pc = 1;
            expectedRunLength = (srcRegister == HL_REGISTER_INDEX || srcRegister == d8_REGISTER_INDEX) ? 2 : 1;

            if (baseInstruction >= 0xC0)
            {
                expected.pc = 2;
                memory->internalRam[0] = baseInstruction;
            }
            else
            {
                memory->internalRam[0] = baseInstruction + srcRegister;
            }
            
            if (baseInstruction == CPU_AND_A_B || baseInstruction == CPU_AND_A_d8)
            {
                expected.a = aValue & srcValue;
                expected.f = GB_FLAGS_H;
                sprintf(instructionName, "AND A (%X) %s (%X)", aValue, registerNames[srcRegister], srcValue);
            } 
            else if (baseInstruction == CPU_XOR_A_B || baseInstruction == CPU_XOR_A_d8)
            {
                expected.a = aValue ^ srcValue;
                sprintf(instructionName, "XOR A (%X) %s (%X)", aValue, registerNames[srcRegister], srcValue);
            } 
            else if (baseInstruction == CPU_OR_A_B || baseInstruction == CPU_OR_A_d8) 
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

            run = runCPU(cpu, memory, 1, 0);

            if (
                !testCPUState(instructionName, testOutput, cpu, &expected) ||
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

int testAND_XOR_OR(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int srcRegister;

    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {       
        if (
            !testSingleBitwise( cpu, memory, testOutput, srcRegister, CPU_AND_A_B) ||
            !testSingleBitwise( cpu, memory, testOutput, srcRegister, CPU_XOR_A_B) ||
            !testSingleBitwise( cpu, memory, testOutput, srcRegister, CPU_OR_A_B) ||
            0
        )
        {
            return 0;
        }
    }

    return 1;
}

int testCP(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int srcRegister;

    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {       
        if (
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_CP_A_B, 0) ||
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_CP_A_B, 1) ||
            0
        )
        {
            return 0;
        }
    }

    return 1;
}

int run0xA_BTests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testCP(cpu, memory, testOutput) &&
        testAND_XOR_OR(cpu, memory, testOutput) &&
        1
    ;
}