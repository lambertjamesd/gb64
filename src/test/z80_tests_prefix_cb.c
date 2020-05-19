#include "z80_test.h"

int testSingleShfit(
    struct Z80State* z80, 
    struct Memory* memory, 
    char* testOutput,
    int srcRegister,
    int baseInstruction,
    int cFlag
) {
    int run;
    unsigned char* srcByte;
    unsigned char* destByte;
    char instructionName[32];
    struct Z80State expected;
    int srcValue;
    int expectedResult;
    int expectedRunLength;

    for (srcValue = 0; srcValue < 0x100; ++srcValue)
    { 
        srcByte = getRegisterPointer(z80, memory->internalRam + 0x20, memory->internalRam + 1, srcRegister);
        destByte = getRegisterPointer(&expected, memory->internalRam + 0x20, memory->internalRam + 1, srcRegister);
        
        initializeZ80(z80);
        z80->h = 0x80;
        z80->l = 0x20;
        z80->f = cFlag ? GB_FLAGS_C : 0;
        expected = *z80;
        expected.pc = 2;
        *srcByte = srcValue;
        expectedRunLength = (srcRegister == HL_REGISTER_INDEX) ? 4 : 2;


        if (baseInstruction == Z80_CB_RLC) 
        {
            expectedResult = (*srcByte << 1) & 0xFE | (*srcByte >> 7) & 0x1;
            expected.f = 
                ((*srcByte & 0x80) ? GB_FLAGS_C : 0) |
                (expectedResult ? 0 : GB_FLAGS_Z)
            ;
            sprintf(instructionName, "RLC %s (%X) C (%d)", registerNames[srcRegister], srcValue, cFlag);
        } 
        else if (baseInstruction == Z80_CB_RRC) 
        {
            expectedResult = (*srcByte >> 1) | (*srcByte << 7) & 0x80;
            expected.f = 
                ((*srcByte & 0x1) ? GB_FLAGS_C : 0) |
                (expectedResult ? 0 : GB_FLAGS_Z)
            ;
            sprintf(instructionName, "RRC %s (%X) C (%d)", registerNames[srcRegister], srcValue, cFlag);
        } 
        else if (baseInstruction == Z80_CB_RL) 
        {
            expectedResult = (*srcByte << 1) & 0xFE | (cFlag ? 0x1 : 0x0);
            expected.f = 
                ((*srcByte & 0x80) ? GB_FLAGS_C : 0) |
                (expectedResult ? 0 : GB_FLAGS_Z)
            ;
            sprintf(instructionName, "RL %s (%X) C (%d)", registerNames[srcRegister], srcValue, cFlag);
        } 
        else if (baseInstruction == Z80_CB_RR) 
        {
            expectedResult = (*srcByte >> 1) | (cFlag ? 0x80 : 0x0);
            expected.f = 
                ((*srcByte & 0x1) ? GB_FLAGS_C : 0) |
                (expectedResult ? 0 : GB_FLAGS_Z)
            ;
            sprintf(instructionName, "RR %s (%X) C (%d)", registerNames[srcRegister], srcValue, cFlag);
        } 
        else if (baseInstruction == Z80_CB_SLA) 
        {
            expectedResult = (*srcByte << 1) & 0xFE;
            expected.f = 
                ((*srcByte & 0x80) ? GB_FLAGS_C : 0) |
                (expectedResult ? 0 : GB_FLAGS_Z)
            ;
            sprintf(instructionName, "SLA %s (%X) C (%d)", registerNames[srcRegister], srcValue, cFlag);
        } 
        else if (baseInstruction == Z80_CB_SRA) 
        {
            expectedResult = (*srcByte >> 1) | (*srcByte & 0x80);
            expected.f = 
                ((*srcByte & 0x1) ? GB_FLAGS_C : 0) |
                (expectedResult ? 0 : GB_FLAGS_Z)
            ;
            sprintf(instructionName, "SRA %s (%X) C (%d)", registerNames[srcRegister], srcValue, cFlag);
        } 
        else if (baseInstruction == Z80_CB_SWAP) 
        {
            expectedResult = (*srcByte << 4) & 0xF0 | (*srcByte >> 4) & 0xF;
            expected.f = expectedResult ? 0 : GB_FLAGS_Z;
            sprintf(instructionName, "SWAP %s (%X) C (%d)", registerNames[srcRegister], srcValue, cFlag);
        } 
        else if (baseInstruction == Z80_CB_SRL) 
        {
            expectedResult = (*srcByte >> 1) & 0x7F;
            expected.f = 
                ((*srcByte & 0x1) ? GB_FLAGS_C : 0) |
                (expectedResult ? 0 : GB_FLAGS_Z)
            ;
            sprintf(instructionName, "SRL %s (%X) C (%d)", registerNames[srcRegister], srcValue, cFlag);
        } 
        else 
        {
            sprintf(testOutput, "Invalid instruction base %X", baseInstruction);
            return 0;
        }

        if (srcByte != destByte)
        {
            *destByte = expectedResult;
        }

        memory->internalRam[0] = Z80_PREFIX_CB;
        memory->internalRam[1] = baseInstruction + srcRegister;

        run = runZ80CPU(z80, memory, 1);
        
        if (
            !testZ80State(instructionName, testOutput, z80, &expected) ||
            !testInt(instructionName, testOutput, run, expectedRunLength) ||
            !testInt(instructionName, testOutput, *destByte, expectedResult)
        )
        {
            return 0;
        }
    }

    return 1;
}

int testShift(
    struct Z80State* z80, 
    struct Memory* memory, 
    char* testOutput,
    int baseInstruction
) {
    int srcRegister;
    
    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {
        if (!testSingleShfit(z80, memory, testOutput, srcRegister, baseInstruction, 0) ||
            !testSingleShfit(z80, memory, testOutput, srcRegister, baseInstruction, 1)
        )
        {
            return 0;
        }
    }

    return 1;
}

int testSingleBitOp(
    struct Z80State* z80, 
    struct Memory* memory, 
    char* testOutput,
    int srcRegister,
    int baseInstruction
) {
    int run;
    unsigned char* srcByte;
    unsigned char* destByte;
    char instructionName[32];
    struct Z80State expected;
    int bit;
    int srcValue;
    int expectedResult;
    int expectedRunLength;

    for (srcValue = 1; srcValue < 0x100; srcValue = srcValue << 1)
    {
        for (bit = 0; bit < 8; ++bit)
        { 
            srcByte = getRegisterPointer(z80, memory->internalRam + 0x20, memory->internalRam + 1, srcRegister);
            destByte = getRegisterPointer(&expected, memory->internalRam + 0x20, memory->internalRam + 1, srcRegister);
            
            initializeZ80(z80);
            z80->h = 0x80;
            z80->l = 0x20;
            expected = *z80;
            expected.pc = 2;
            *srcByte = srcValue;
            expectedRunLength = (srcRegister == HL_REGISTER_INDEX) ? 4 : 2;


            if (baseInstruction == Z80_CB_BIT) 
            {
                expectedResult = *srcByte;
                expected.f = GB_FLAGS_H | ((expectedResult & (1 << bit)) ? 0 : GB_FLAGS_Z);
                sprintf(instructionName, "BIT %d %s (%X)", bit, registerNames[srcRegister], srcValue);
            }
            else if (baseInstruction == Z80_CB_RES) 
            {
                expectedResult = *srcByte & ~(1 << bit);
                sprintf(instructionName, "RES %d %s (%X)", bit, registerNames[srcRegister], srcValue);
            }
            else if (baseInstruction == Z80_CB_SET) 
            {
                expectedResult = *srcByte | (1 << bit);
                sprintf(instructionName, "SET %d %s (%X)", bit, registerNames[srcRegister], srcValue);
            }
            else 
            {
                sprintf(testOutput, "Invalid instruction base %X", baseInstruction);
                return 0;
            }

            if (srcByte != destByte)
            {
                *destByte = expectedResult;
            }

            memory->internalRam[0] = Z80_PREFIX_CB;
            memory->internalRam[1] = baseInstruction + srcRegister + bit * 8;

            run = runZ80CPU(z80, memory, 1);
            
            if (
                !testZ80State(instructionName, testOutput, z80, &expected) ||
                !testInt(instructionName, testOutput, run, expectedRunLength) ||
                !testInt(instructionName, testOutput, *destByte, expectedResult)
            )
            {
                return 0;
            }
        }
    }

    return 1;
}

int testBitOp(
    struct Z80State* z80, 
    struct Memory* memory, 
    char* testOutput,
    int baseInstruction
) {
    int srcRegister;
    
    for (srcRegister = 0; srcRegister < REGISTER_COUNT; ++srcRegister)
    {
        if (!testSingleBitOp(z80, memory, testOutput, srcRegister, baseInstruction))
        {
            return 0;
        }
    }

    return 1;
}

int runPrefixCBTests(struct Z80State* z80, struct Memory* memory, char* testOutput)
{
    return 
        testShift(z80, memory, testOutput, Z80_CB_RLC) &&
        testShift(z80, memory, testOutput, Z80_CB_RRC) &&
        testShift(z80, memory, testOutput, Z80_CB_RL) &&
        testShift(z80, memory, testOutput, Z80_CB_RR) &&
        testShift(z80, memory, testOutput, Z80_CB_SLA) &&
        testShift(z80, memory, testOutput, Z80_CB_SRA) &&
        testShift(z80, memory, testOutput, Z80_CB_SWAP) &&
        testShift(z80, memory, testOutput, Z80_CB_SRL) &&
        testBitOp(z80, memory, testOutput, Z80_CB_BIT) &&
        testBitOp(z80, memory, testOutput, Z80_CB_RES) &&
        testBitOp(z80, memory, testOutput, Z80_CB_SET) &&
    1;
}