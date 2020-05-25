
#include "cpu_test.h"

int testSingleADD(
    struct CPUState* cpu, 
    struct Memory* memory, 
    char* testOutput,
    int srcRegister,
    int baseInstruction,
    int cFlag
) {
    int run;
    unsigned char* srcByte;
    char instructionName[32];
    struct CPUState expected;
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

            srcByte = getRegisterPointer(cpu, memory->internalRam + 0x20, memory->internalRam + 1, srcRegister);

            if (baseInstruction == CPU_ADD_A_B || baseInstruction == CPU_ADC_A_B || 
                baseInstruction == CPU_ADD_A_d8 || baseInstruction == CPU_ADC_A_d8)
            {
                aResult = aValue + srcValue;
            }
            else
            {
                aResult = aValue - srcValue;
            }

            if ((baseInstruction == CPU_ADC_A_B || baseInstruction == CPU_ADC_A_d8) && cFlag)
            {
                ++aResult;
            }
            else if ((baseInstruction == CPU_SBC_A_B || (baseInstruction == CPU_SBC_A_d8)) && cFlag)
            {
                --aResult;
            }

            initializeCPU(cpu);
            cpu->a = aValue;
            cpu->h = 0x80;
            cpu->l = 0x20;
            *srcByte = srcValue;

            if (cFlag)
            {
                cpu->f = GB_FLAGS_C;
            }

            expected = *cpu;
            expected.pc = (srcRegister == d8_REGISTER_INDEX) ? 2 : 1;
            if (baseInstruction != CPU_CP_A_B && baseInstruction != CPU_CP_A_d8)
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

            if (baseInstruction == CPU_SUB_A_B || baseInstruction == CPU_SBC_A_B || baseInstruction == CPU_CP_A_B ||
                baseInstruction == CPU_SUB_A_d8 || baseInstruction == CPU_SBC_A_d8 || baseInstruction == CPU_CP_A_d8)
            {
                expected.f |= GB_FLAGS_N;
            }

            if (baseInstruction < 0xC0)
            {            
                memory->internalRam[0] = baseInstruction + srcRegister;
            }
            else
            {
                memory->internalRam[0] = baseInstruction;
            }

            run = runCPUCPU(cpu, memory, 1);

            if (baseInstruction == CPU_ADD_A_B || baseInstruction == CPU_ADD_A_d8) 
            {
                sprintf(instructionName, "ADD A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else if (baseInstruction == CPU_ADC_A_B || baseInstruction == CPU_ADC_A_d8) 
            {
                sprintf(instructionName, "ADC A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else if (baseInstruction == CPU_SUB_A_B || baseInstruction == CPU_SUB_A_d8) 
            {
                sprintf(instructionName, "SUB A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else if (baseInstruction == CPU_SBC_A_B || baseInstruction == CPU_SBC_A_d8) 
            {
                sprintf(instructionName, "SBC A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else if (baseInstruction == CPU_CP_A_B || baseInstruction == CPU_CP_A_d8) 
            {
                sprintf(instructionName, "CP A (%X) %s (%X) C (%d)", aValue, registerNames[srcRegister], srcValue, cFlag);
            } 
            else 
            {
                sprintf(testOutput, "Invalid instruction base %X", baseInstruction);
                return 0;
            }

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

int testADD_ADC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int srcRegister;

    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {       
        if (
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_ADD_A_B, 0) ||
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_ADD_A_B, 1) ||
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_ADC_A_B, 0) ||
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_ADC_A_B, 1) ||
            0
        )
        {
            return 0;
        }
    }

    return 1;
}

int testSUB_SBC(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int srcRegister;

    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {       
        if (
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_SUB_A_B, 0) ||
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_SUB_A_B, 1) ||
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_SBC_A_B, 0) ||
            !testSingleADD( cpu, memory, testOutput, srcRegister, CPU_SBC_A_B, 1) ||
            0
        )
        {
            return 0;
        }
    }

    return 1;
}

int run0x8_9Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testADD_ADC(cpu, memory, testOutput) &&
        testSUB_SBC(cpu, memory, testOutput) &&
        1
    ;
}