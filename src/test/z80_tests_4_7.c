
#include "z80_test.h"

int testLD_X_Y(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    int expectedRunLength;
    struct Z80State expected;
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
            
            srcByte = getRegisterPointer(z80, memory + 0x20, memory + 1, srcRegister);
            targetByte = getRegisterPointer(&expected, memory + 0x20, memory + 1, targetRegister);

            initializeZ80(z80);
            memory[0x20] = 0x0;
            z80->h = 0x80;
            z80->l = 0x20;
            *srcByte = 0x63;
            expected = *z80;
            expected.pc = 1;
            *targetByte = 0x63;
            expectedRunLength = (srcRegister == HL_REGISTER_INDEX || targetRegister == HL_REGISTER_INDEX) ? 2 : 1;

            memory[0] = Z80_LD_B_B + REGISTER_COUNT * targetRegister + srcRegister;

            run = runZ80CPU(z80, memoryMap, 1);

            sprintf(instructionName, "LD %s %s", registerNames[targetRegister], registerNames[srcRegister]);

            if (
                !testZ80State(instructionName, testOutput, z80, &expected) ||
                !testInt(instructionName, testOutput, run, expectedRunLength) ||
                !testInt(instructionName, testOutput, *targetByte, 0x63)
            )
            {
                return 0;
            }
        }
    }

    return 1;
}

int testHALT(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    int run;
    struct Z80State expected;
    initializeZ80(z80);
    expected = *z80;
    expected.pc = 1;
    expected.stopReason = STOP_REASON_HALT;

    memory[0] = Z80_HALT;

    run = runZ80CPU(z80, memoryMap, 1);

    return 
        testZ80State("HALT", testOutput, z80, &expected) &&
        testInt("HALT run result", testOutput, run, 1) &&
        1
    ;
}

int run0x4_7Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput)
{
    return 
        testLD_X_Y(z80, memoryMap, memory, testOutput) &&
        testHALT(z80, memoryMap, memory, testOutput) &&
        1
    ;
}