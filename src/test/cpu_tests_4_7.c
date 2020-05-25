
#include "cpu_test.h"

int testLD_X_Y(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    int expectedRunLength;
    int expectedValue;
    struct CPUState expected;
    int targetRegister;
    int srcRegister;
    unsigned char* targetByte;
    unsigned char* srcByte;
    char instructionName[10];

    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {
        for (targetRegister = 0; targetRegister < REGISTER_COUNT; ++targetRegister)
        {
            if (srcRegister == HL_REGISTER_INDEX && targetRegister == HL_REGISTER_INDEX)
            {
                continue;
            }

            if (srcRegister == H_REGISTER_INDEX)
            {
                expectedValue = 0x80;
            }
            else if (srcRegister == L_REGISTER_INDEX)
            {
                expectedValue = 0x20;
            }
            else
            {
                expectedValue = 0x63;
            }
            
            srcByte = getRegisterPointer(cpu, memory->internalRam + 0x20, memory->internalRam + 1, srcRegister);
            targetByte = getRegisterPointer(&expected, memory->internalRam + 0x20, memory->internalRam + 1, targetRegister);

            initializeCPU(cpu);
            memory->internalRam[0x20] = 0x0;
            cpu->h = 0x80;
            cpu->l = 0x20;
            *srcByte = expectedValue;
            expected = *cpu;
            expected.pc = 1;
            *targetByte = expectedValue;
            expectedRunLength = (srcRegister == HL_REGISTER_INDEX || targetRegister == HL_REGISTER_INDEX) ? 2 : 1;

            memory->internalRam[0] = CPU_LD_B_B + REGISTER_COUNT * targetRegister + srcRegister;

            run = runCPU(cpu, memory, 1);

            sprintf(instructionName, "LD %s %s", registerNames[targetRegister], registerNames[srcRegister]);

            if (
                !testCPUState(instructionName, testOutput, cpu, &expected) ||
                !testInt(instructionName, testOutput, run, expectedRunLength) ||
                !testInt(instructionName, testOutput, *targetByte, expectedValue)
            )
            {
                return 0;
            }
        }
    }

    return 1;
}

int testHALT(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    int run;
    struct CPUState expected;
    initializeCPU(cpu);
    expected = *cpu;
    expected.pc = 1;
    expected.stopReason = STOP_REASON_HALT;

    memory->internalRam[0] = CPU_HALT;

    run = runCPU(cpu, memory, 1);

    return 
        testCPUState("HALT", testOutput, cpu, &expected) &&
        testInt("HALT run result", testOutput, run, 1) &&
        1
    ;
}

int run0x4_7Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return 
        testLD_X_Y(cpu, memory, testOutput) &&
        // testHALT(cpu, memory, testOutput) &&
        1
    ;
}