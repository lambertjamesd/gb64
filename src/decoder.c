#include "decoder.h"
#include "bool.h"
#include "instructions.h"

struct InstructionInformation baseInstructions[0x100] = {
    {"NOP", 0, 0},
    {"LD BC, %04Xh", 2, 0},
    {"LD (BC), A", 0, 0},
    {"INC BC", 0, 0},
    {"INC B", 0, 0},
    {"DEC B", 0, 0},
    {"LD B, %02Xh", 1, 0},
    {"RLCA", 0, 0},
    {"LD (%04Xh), SP", 2, 0},
    {"ADD HL, BC", 0, 0},
    {"LD A, (BC)", 0, 0},
    {"DEC BC", 0, 0},
    {"INC C", 0, 0},
    {"DEC C", 0, 0},
    {"LD C, (%02Xh)", 1, 0},
    {"RRCA", 0, 0},

    {"STOP", 0, 0},
    {"LD DE, %04Xh", 2, 0},
    {"LD (DE), A", 0, 0},
    {"INC DE", 0, 0},
    {"INC D", 0, 0},
    {"DEC D", 0, 0},
    {"LD D, %02Xh", 1, 0},
    {"RLA", 0, 0},
    {"JR %04Xh", 1, INST_FLAGS_RELATIVE_ADDRESS},
    {"ADD HL, DE", 0, 0},
    {"LD A, (DE)", 0, 0},
    {"DEC DE", 0, 0},
    {"INC E", 0, 0},
    {"DEC E", 0, 0},
    {"LD E, (%02Xh)", 1, 0},
    {"RRA", 0, 0},

    {"JR NZ, %04Xh", 1, INST_FLAGS_RELATIVE_ADDRESS | INST_FLAGS_BRANCH},
    {"LD HL, %04Xh", 2, 0},
    {"LDI (HL), A", 0, 0},
    {"INC HL", 0, 0},
    {"INC H", 0, 0},
    {"DEC H", 0, 0},
    {"LD H, %02Xh", 1, 0},
    {"DAA", 0, 0},
    {"JR Z, %04Xh", 1, INST_FLAGS_RELATIVE_ADDRESS | INST_FLAGS_BRANCH},
    {"ADD HL, HL", 0, 0},
    {"LDI A, (HL)", 0, 0},
    {"DEC HL", 0, 0},
    {"INC H", 0, 0},
    {"DEC L", 0, 0},
    {"LD L, (%02Xh)", 1, 0},
    {"CPL", 0, 0},

    {"JR NC, %04Xh", 1, INST_FLAGS_RELATIVE_ADDRESS | INST_FLAGS_BRANCH},
    {"LD SP, %04Xh", 2, 0},
    {"LDD (HL), A", 0, 0},
    {"INC SP", 0, 0},
    {"INC (HL)", 0, 0},
    {"DEC (HL)", 0, 0},
    {"LD (HL), %02Xh", 1, 0},
    {"SCF", 0, 0},
    {"JR C, %04Xh", 1, INST_FLAGS_RELATIVE_ADDRESS | INST_FLAGS_BRANCH},
    {"ADD HL, SP", 0, 0},
    {"LDD A, (HL)", 0, 0},
    {"DEC SP", 0, 0},
    {"INC A", 0, 0},
    {"DEC A", 0, 0},
    {"LD A, (%02Xh)", 1, 0},
    {"CCF", 0, 0},
    
    {"LD B, B", 0, 0},
    {"LD B, C", 0, 0},
    {"LD B, D", 0, 0},
    {"LD B, E", 0, 0},
    {"LD B, H", 0, 0},
    {"LD B, L", 0, 0},
    {"LD B, (HL)", 0, 0},
    {"LD B, A", 0, 0},
    {"LD C, B", 0, 0},
    {"LD C, C", 0, 0},
    {"LD C, D", 0, 0},
    {"LD C, E", 0, 0},
    {"LD C, H", 0, 0},
    {"LD C, L", 0, 0},
    {"LD C, (HL)", 0, 0},
    {"LD C, A", 0, 0},
    
    {"LD D, B", 0, 0},
    {"LD D, C", 0, 0},
    {"LD D, D", 0, 0},
    {"LD D, E", 0, 0},
    {"LD D, H", 0, 0},
    {"LD D, L", 0, 0},
    {"LD D, (HL)", 0, 0},
    {"LD D, A", 0, 0},
    {"LD E, B", 0, 0},
    {"LD E, C", 0, 0},
    {"LD E, D", 0, 0},
    {"LD E, E", 0, 0},
    {"LD E, H", 0, 0},
    {"LD E, L", 0, 0},
    {"LD E, (HL)", 0, 0},
    {"LD E, A", 0, 0},
    
    {"LD H, B", 0, 0},
    {"LD H, C", 0, 0},
    {"LD H, D", 0, 0},
    {"LD H, E", 0, 0},
    {"LD H, H", 0, 0},
    {"LD H, L", 0, 0},
    {"LD H, (HL)", 0, 0},
    {"LD H, A", 0, 0},
    {"LD L, B", 0, 0},
    {"LD L, C", 0, 0},
    {"LD L, D", 0, 0},
    {"LD L, E", 0, 0},
    {"LD L, H", 0, 0},
    {"LD L, L", 0, 0},
    {"LD L, (HL)", 0, 0},
    {"LD L, A", 0, 0},
    
    {"LD (HL), B", 0, 0},
    {"LD (HL), C", 0, 0},
    {"LD (HL), D", 0, 0},
    {"LD (HL), E", 0, 0},
    {"LD (HL), H", 0, 0},
    {"LD (HL), L", 0, 0},
    {"HALT", 0, 0},
    {"LD (HL), A", 0, 0},
    {"LD A, B", 0, 0},
    {"LD A, C", 0, 0},
    {"LD A, D", 0, 0},
    {"LD A, E", 0, 0},
    {"LD A, H", 0, 0},
    {"LD A, L", 0, 0},
    {"LD A, (HL)", 0, 0},
    {"LD A, A", 0, 0},
    
    {"ADD B", 0, 0},
    {"ADD C", 0, 0},
    {"ADD D", 0, 0},
    {"ADD E", 0, 0},
    {"ADD H", 0, 0},
    {"ADD L", 0, 0},
    {"ADD (HL)", 0, 0},
    {"ADD A", 0, 0},
    {"ADC B", 0, 0},
    {"ADC C", 0, 0},
    {"ADC D", 0, 0},
    {"ADC E", 0, 0},
    {"ADC H", 0, 0},
    {"ADC L", 0, 0},
    {"ADC (HL)", 0, 0},
    {"ADC A", 0, 0},
    
    {"SUB B", 0, 0},
    {"SUB C", 0, 0},
    {"SUB D", 0, 0},
    {"SUB E", 0, 0},
    {"SUB H", 0, 0},
    {"SUB L", 0, 0},
    {"SUB (HL)", 0, 0},
    {"SUB A", 0, 0},
    {"SBC B", 0, 0},
    {"SBC C", 0, 0},
    {"SBC D", 0, 0},
    {"SBC E", 0, 0},
    {"SBC H", 0, 0},
    {"SBC L", 0, 0},
    {"SBC (HL)", 0, 0},
    {"SBC A", 0, 0},
    
    {"AND B", 0, 0},
    {"AND C", 0, 0},
    {"AND D", 0, 0},
    {"AND E", 0, 0},
    {"AND H", 0, 0},
    {"AND L", 0, 0},
    {"AND (HL)", 0, 0},
    {"AND A", 0, 0},
    {"XOR B", 0, 0},
    {"XOR C", 0, 0},
    {"XOR D", 0, 0},
    {"XOR E", 0, 0},
    {"XOR H", 0, 0},
    {"XOR L", 0, 0},
    {"XOR (HL)", 0, 0},
    {"XOR A", 0, 0},
    
    {"OR B", 0, 0},
    {"OR C", 0, 0},
    {"OR D", 0, 0},
    {"OR E", 0, 0},
    {"OR H", 0, 0},
    {"OR L", 0, 0},
    {"OR (HL)", 0, 0},
    {"OR A", 0, 0},
    {"CP B", 0, 0},
    {"CP C", 0, 0},
    {"CP D", 0, 0},
    {"CP E", 0, 0},
    {"CP H", 0, 0},
    {"CP L", 0, 0},
    {"CP (HL)", 0, 0},
    {"CP A", 0, 0},
    
    {"RET NZ", 0, INST_FLAGS_RETURN | INST_FLAGS_BRANCH},
    {"POP BC", 0, 0},
    {"JP NZ, %04Xh", 2, INST_FLAGS_ADDRESS | INST_FLAGS_BRANCH},
    {"JP %04Xh", 2, INST_FLAGS_ADDRESS},
    {"CALL NZ, %04Xh", 2, INST_FLAGS_ADDRESS | INST_FLAGS_BRANCH},
    {"PUSH BC", 0, 0},
    {"ADD %02Xh", 1, 0},
    {"RST 00h", 0, INST_FLAGS_BRANCH},
    {"RET Z", 0, INST_FLAGS_RETURN | INST_FLAGS_BRANCH},
    {"RET", 0, INST_FLAGS_RETURN},
    {"JP Z, %04Xh", 2, INST_FLAGS_ADDRESS | INST_FLAGS_BRANCH},
    {"PREFIX CB", 1, 0},
    {"CALL Z, %04Xh", 2, INST_FLAGS_ADDRESS | INST_FLAGS_BRANCH},
    {"CALL %04Xh", 2, INST_FLAGS_ADDRESS | INST_FLAGS_BRANCH},
    {"ADC %02Xh", 1, 0},
    {"RST 08h", 0, INST_FLAGS_BRANCH},
    
    {"RET NC", 0, INST_FLAGS_RETURN | INST_FLAGS_BRANCH},
    {"POP DE", 0, 0},
    {"JP NC, %04Xh", 2, INST_FLAGS_ADDRESS | INST_FLAGS_BRANCH},
    {"!error (D3h)", 0, 0},
    {"CALL NC, %04Xh", 2, INST_FLAGS_ADDRESS | INST_FLAGS_BRANCH},
    {"PUSH DE", 0, 0},
    {"SUB %02Xh", 1, 0},
    {"RST 10h", 0, INST_FLAGS_BRANCH},
    {"RET C", 0, INST_FLAGS_RETURN | INST_FLAGS_BRANCH},
    {"RETI", 0, INST_FLAGS_RETURN},
    {"JP C, %04Xh", 2, INST_FLAGS_ADDRESS | INST_FLAGS_BRANCH},
    {"!error (DBh)", 0, 0},
    {"CALL C, %04Xh", 2, INST_FLAGS_ADDRESS | INST_FLAGS_BRANCH},
    {"!error (DDh)", 0, 0},
    {"SBC %02Xh", 1, 0},
    {"RST 18h", 0, INST_FLAGS_BRANCH},
    
    {"LDH (0xFF%02X), A", 1, 0},
    {"POP HL", 0, 0},
    {"LDH (C), A", 0, 0},
    {"!error (E3h)", 0, 0},
    {"!error (E4h)", 0, 0},
    {"PUSH HL", 0, 0},
    {"AND %02Xh", 1, 0},
    {"RST 20h", 0, INST_FLAGS_BRANCH},
    {"ADD SP, %d", 1, 0},
    {"JP (HL)", 0, 0},
    {"LD (%04Xh), A", 2, 0},
    {"!error (EBh)", 0, 0},
    {"!error (ECh)", 0, 0},
    {"!error (EDh)", 0, 0},
    {"XOR %02Xh", 1, 0},
    {"RST 28h", 0, INST_FLAGS_BRANCH},
    
    {"LDH A, (0xFF%02X)", 1, 0},
    {"POP AF", 0, 0},
    {"LDH A, (C)", 0, 0},
    {"DI", 0, 0},
    {"!error (F4h)", 0, 0},
    {"PUSH AF", 0, 0},
    {"OR %02Xh", 1, 0},
    {"RST 30h", 0, INST_FLAGS_BRANCH},
    {"LD HL,SP+%d", 1, 0},
    {"LD SP, HL", 0, 0},
    {"LD A, (%04Xh)", 2, 0},
    {"EI", 0, 0},
    {"!error (ECh)", 0, 0},
    {"!error (EDh)", 0, 0},
    {"CP %02Xh", 1, 0},
    {"RST 38h", 0, INST_FLAGS_BRANCH},
};

char* prefixInstructionNames[] = {
    "RLC", "RRC",
    "RL", "RR",
    "SLA", "SRA",
    "SWAP", "SRL",
    "BIT 0", "BIT 1",
    "BIT 2", "BIT 3",
    "BIT 4", "BIT 5",
    "BIT 6", "BIT 7",
    "RES 0", "RES 1",
    "RES 2", "RES 3",
    "RES 4", "RES 5",
    "RES 6", "RES 7",
    "SET 0", "SET 1",
    "SET 2", "SET 3",
    "SET 4", "SET 5",
    "SET 6", "SET 7",
};

char* prefxInstructionTargets[] = {
    "B", "C",
    "D", "E",
    "H", "L",
    "(HL)", "A",
};

int decodeInstruction(char* output, u8* input, u16 address)
{
    if (*input == CPU_PREFIX_CB)
    {  
        u8 prefixInstruction = input[1];
        sprintf(output, "%s %s", prefixInstructionNames[prefixInstruction >> 3], prefxInstructionTargets[prefixInstruction & 0x7]);
        return 2;
    }
    else
    {
        int inputParameter = 0;
        struct InstructionInformation* instruction = &baseInstructions[input[0]];

        if (instruction->flags & INST_FLAGS_RELATIVE_ADDRESS)
        {
            inputParameter = address + 2 + (s8)input[1];
        }
        else if (instruction->extraBytes == 1)
        {
            inputParameter = input[1];
        }
        else if (instruction->extraBytes == 2)
        {
            inputParameter = (int)input[1] | ((int)input[2] << 8);
        }

        sprintf(output, instruction->formatString, inputParameter);

        return instruction->extraBytes + 1;
    }
}

int getInstructionSize(u8 instructionValue)
{
    return baseInstructions[instructionValue].extraBytes + 1;
}

u8* scanForInstruction(u8* input, u8* upperBound, u8 instruction)
{
    while (*input != instruction && input < upperBound)
    {
        int size = getInstructionSize(*input);

        if (input + size > upperBound)
        {
            return input;
        }

        input += size;
    }

    return input;
}

struct AddressTuple getInstructionBranch(u8* instructionData, u16 address, u16 topOfStack, u16 hl)
{
    struct InstructionInformation* instruction = &baseInstructions[instructionData[0]];
    struct AddressTuple result;
    result.nextInstruction = address + instruction->extraBytes + 1;

    if (instruction->flags & INST_FLAGS_ADDRESS)
    {
        result.branchInstruction = (int)instructionData[1] | ((int)instructionData[2] << 8);
    }
    else if (instruction->flags & INST_FLAGS_RELATIVE_ADDRESS)
    {
        result.branchInstruction = address + 2 + (s8)instructionData[1];
    }
    else if (instruction->flags & INST_FLAGS_RETURN)
    {
        result.branchInstruction = topOfStack;
    }
    else if (instructionData[0] == CPU_JP_HL)
    {
        result.branchInstruction = hl;
    }
    else if (instructionData[0] == CPU_RST_00H)
    {
        result.branchInstruction = 0x0000;
    }
    else if (instructionData[0] == CPU_RST_08H)
    {
        result.branchInstruction = 0x0000;
    }
    else if (instructionData[0] == CPU_RST_10H)
    {
        result.branchInstruction = 0x0000;
    }
    else if (instructionData[0] == CPU_RST_18H)
    {
        result.branchInstruction = 0x0000;
    }
    else if (instructionData[0] == CPU_RST_20H)
    {
        result.branchInstruction = 0x0000;
    }
    else if (instructionData[0] == CPU_RST_28H)
    {
        result.branchInstruction = 0x0000;
    }
    else if (instructionData[0] == CPU_RST_30H)
    {
        result.branchInstruction = 0x0000;
    }
    else if (instructionData[0] == CPU_RST_38H)
    {
        result.branchInstruction = 0x0000;
    }
    else
    {
        result.branchInstruction = result.nextInstruction;
    }

    if (!(instruction->flags & INST_FLAGS_BRANCH))
    {
        result.nextInstruction = result.branchInstruction;
    }

    return result;
}

bool isInstructionCall(u8 instruction)
{
    switch (instruction)
    {
        case CPU_CALL_NZ:
        case CPU_CALL_Z:
        case CPU_CALL:
        case CPU_CALL_NC:
        case CPU_CALL_C:
        case CPU_RST_00H:
        case CPU_RST_08H:
        case CPU_RST_10H:
        case CPU_RST_18H:
        case CPU_RST_20H:
        case CPU_RST_28H:
        case CPU_RST_30H:
        case CPU_RST_38H:
            return TRUE;
    }

    return FALSE;
}