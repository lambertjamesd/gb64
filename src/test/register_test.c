#include "cpu_test.h"

int runJOYPTests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    initializeCPU(cpu);
    cpu->a = 0x10;

    WRITE_REGISTER_DIRECT(memory, _REG_JOYSTATE, 0xAB);

    memory->internalRam[0] = CPU_LDH_a8_A;
    memory->internalRam[1] = 0x00;
    memory->internalRam[2] = CPU_LD_A_d8;
    memory->internalRam[3] = 0x20;
    memory->internalRam[4] = CPU_LDH_a8_A;
    memory->internalRam[5] = 0x00;

    runCPUCPU(cpu, memory, 2);

    if (!testInt("JOYP", testOutput, 
        READ_REGISTER_DIRECT(memory, REG_JOYP), 
        0x1A)
    )
    {
        return 0;
    }
    
    runCPUCPU(cpu, memory, 4);

    return testInt("JOYP select second", testOutput, 
        READ_REGISTER_DIRECT(memory, REG_JOYP), 
        0x2B
    );
}

int runDIVTests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    initializeCPU(cpu);
    cpu->a = 0;

    WRITE_REGISTER_DIRECT(memory, _REG_JOYSTATE, 0xAB);

    memory->internalRam[0] = CPU_INC_A;
    memory->internalRam[1] = CPU_JR_NZ;
    memory->internalRam[2] = -3;
    memory->internalRam[3] = CPU_INC_A;
    memory->internalRam[4] = CPU_LDH_a8_A;
    memory->internalRam[5] = 0x04;
    memory->internalRam[6] = CPU_JR_NZ;
    memory->internalRam[7] = -5;

    runCPUCPU(cpu, memory, 512);

    if (!testInt("DIV", testOutput, 
        READ_REGISTER_DIRECT(memory, REG_DIV), 
        0x8)
    )
    {
        return 0;
    }

    cpu->pc = 3;
    
    runCPUCPU(cpu, memory, 512);

    return testInt("DIV select second", testOutput, 
        READ_REGISTER_DIRECT(memory, REG_DIV), 
        0x1
    );
}

int runSVBKTests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    initializeCPU(cpu);
    cpu->a = 0x3;

    memory->internalRam[0] = CPU_LDH_a8_A;
    memory->internalRam[1] = 0x70;
    memory->internalRam[2] = CPU_LD_A_d8;
    memory->internalRam[3] = 0;
    memory->internalRam[4] = CPU_LDH_a8_A;
    memory->internalRam[5] = 0x70;

    runCPUCPU(cpu, memory, 2);

    if (!testInt("SVBK ram bank", testOutput, 
        (int)memory->memoryMap[MEMORY_RAM_BANK_INDEX], 
        (int)memory->internalRam + 0x3000)
    )
    {
        return 0;
    }
    
    runCPUCPU(cpu, memory, 4);

    return testInt("SVBK ram bank 0 bank", testOutput, 
        (int)memory->memoryMap[MEMORY_RAM_BANK_INDEX], 
        (int)memory->internalRam + 0x1000
    );
}

int runRegisterTests(struct CPUState* cpu, struct Memory* memory, char* testOutput)
{
    return
        runJOYPTests(cpu, memory, testOutput) &&
        runDIVTests(cpu, memory, testOutput) &&
        runSVBKTests(cpu, memory, testOutput) &&
    1;
}