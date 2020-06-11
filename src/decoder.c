#include "decoder.h"
#include "bool.h"
#include "instructions.h"

struct InstructionInformation baseInstructions[0x100] = {
    {"NOP", 0, FALSE},
    {"LD BC, %04Xh", 2, FALSE},
    {"LD (BC), A", 0, FALSE},
    {"INC BC", 0, FALSE},
    {"INC B", 0, FALSE},
    {"DEC B", 0, FALSE},
    {"LD B, %02Xh", 1, FALSE},
    {"RLCA", 0, FALSE},
    {"LD (%04Xh), SP", 2, FALSE},
    {"ADD HL, BC", 0, FALSE},
    {"LD A, (BC)", 0, FALSE},
    {"DEC BC", 0, FALSE},
    {"INC C", 0, FALSE},
    {"DEC C", 0, FALSE},
    {"LD C, (%02Xh)", 1, FALSE},
    {"RRCA", 0, FALSE},

    {"STOP", 0, FALSE},
    {"LD DE, %04Xh", 2, FALSE},
    {"LD (DE), A", 0, FALSE},
    {"INC DE", 0, FALSE},
    {"INC D", 0, FALSE},
    {"DEC D", 0, FALSE},
    {"LD D, %02Xh", 1, FALSE},
    {"RLA", 0, FALSE},
    {"JR %04Xh", 1, TRUE},
    {"ADD HL, DE", 0, FALSE},
    {"LD A, (DE)", 0, FALSE},
    {"DEC DE", 0, FALSE},
    {"INC E", 0, FALSE},
    {"DEC E", 0, FALSE},
    {"LD E, (%02Xh)", 1, FALSE},
    {"RRA", 0, FALSE},

    {"JR NZ, %04Xh", 1, TRUE},
    {"LD HL, %04Xh", 2, FALSE},
    {"LDI (HL), A", 0, FALSE},
    {"INC HL", 0, FALSE},
    {"INC H", 0, FALSE},
    {"DEC H", 0, FALSE},
    {"LD H, %02Xh", 1, FALSE},
    {"DAA", 0, FALSE},
    {"JR Z, %04Xh", 1, TRUE},
    {"ADD HL, HL", 0, FALSE},
    {"LDI A, (HL)", 0, FALSE},
    {"DEC HL", 0, FALSE},
    {"INC H", 0, FALSE},
    {"DEC L", 0, FALSE},
    {"LD L, (%02Xh)", 1, FALSE},
    {"CPL", 0, FALSE},

    {"JR NC, %04Xh", 1, TRUE},
    {"LD SP, %04Xh", 2, FALSE},
    {"LDD (HL), A", 0, FALSE},
    {"INC SP", 0, FALSE},
    {"INC (HL)", 0, FALSE},
    {"DEC (HL)", 0, FALSE},
    {"LD (HL), %02Xh", 1, FALSE},
    {"SCF", 0, FALSE},
    {"JR C, %04Xh", 1, TRUE},
    {"ADD HL, SP", 0, FALSE},
    {"LDD A, (HL)", 0, FALSE},
    {"DEC SP", 0, FALSE},
    {"INC A", 0, FALSE},
    {"DEC A", 0, FALSE},
    {"LD A, (%02Xh)", 1, FALSE},
    {"CCF", 0, FALSE},
    
    {"LD B, B", 0, FALSE},
    {"LD B, C", 0, FALSE},
    {"LD B, D", 0, FALSE},
    {"LD B, E", 0, FALSE},
    {"LD B, H", 0, FALSE},
    {"LD B, L", 0, FALSE},
    {"LD B, (HL)", 0, FALSE},
    {"LD B, A", 0, FALSE},
    {"LD C, B", 0, FALSE},
    {"LD C, C", 0, FALSE},
    {"LD C, D", 0, FALSE},
    {"LD C, E", 0, FALSE},
    {"LD C, H", 0, FALSE},
    {"LD C, L", 0, FALSE},
    {"LD C, (HL)", 0, FALSE},
    {"LD C, A", 0, FALSE},
    
    {"LD D, B", 0, FALSE},
    {"LD D, C", 0, FALSE},
    {"LD D, D", 0, FALSE},
    {"LD D, E", 0, FALSE},
    {"LD D, H", 0, FALSE},
    {"LD D, L", 0, FALSE},
    {"LD D, (HL)", 0, FALSE},
    {"LD D, A", 0, FALSE},
    {"LD E, B", 0, FALSE},
    {"LD E, C", 0, FALSE},
    {"LD E, D", 0, FALSE},
    {"LD E, E", 0, FALSE},
    {"LD E, H", 0, FALSE},
    {"LD E, L", 0, FALSE},
    {"LD E, (HL)", 0, FALSE},
    {"LD E, A", 0, FALSE},
    
    {"LD H, B", 0, FALSE},
    {"LD H, C", 0, FALSE},
    {"LD H, D", 0, FALSE},
    {"LD H, E", 0, FALSE},
    {"LD H, H", 0, FALSE},
    {"LD H, L", 0, FALSE},
    {"LD H, (HL)", 0, FALSE},
    {"LD H, A", 0, FALSE},
    {"LD L, B", 0, FALSE},
    {"LD L, C", 0, FALSE},
    {"LD L, D", 0, FALSE},
    {"LD L, E", 0, FALSE},
    {"LD L, H", 0, FALSE},
    {"LD L, L", 0, FALSE},
    {"LD L, (HL)", 0, FALSE},
    {"LD L, A", 0, FALSE},
    
    {"LD (HL), B", 0, FALSE},
    {"LD (HL), C", 0, FALSE},
    {"LD (HL), D", 0, FALSE},
    {"LD (HL), E", 0, FALSE},
    {"LD (HL), H", 0, FALSE},
    {"LD (HL), L", 0, FALSE},
    {"HALT", 0, FALSE},
    {"LD (HL), A", 0, FALSE},
    {"LD A, B", 0, FALSE},
    {"LD A, C", 0, FALSE},
    {"LD A, D", 0, FALSE},
    {"LD A, E", 0, FALSE},
    {"LD A, H", 0, FALSE},
    {"LD A, L", 0, FALSE},
    {"LD A, (HL)", 0, FALSE},
    {"LD A, A", 0, FALSE},
    
    {"ADD B", 0, FALSE},
    {"ADD C", 0, FALSE},
    {"ADD D", 0, FALSE},
    {"ADD E", 0, FALSE},
    {"ADD H", 0, FALSE},
    {"ADD L", 0, FALSE},
    {"ADD (HL)", 0, FALSE},
    {"ADD A", 0, FALSE},
    {"ADC B", 0, FALSE},
    {"ADC C", 0, FALSE},
    {"ADC D", 0, FALSE},
    {"ADC E", 0, FALSE},
    {"ADC H", 0, FALSE},
    {"ADC L", 0, FALSE},
    {"ADC (HL)", 0, FALSE},
    {"ADC A", 0, FALSE},
    
    {"SUB B", 0, FALSE},
    {"SUB C", 0, FALSE},
    {"SUB D", 0, FALSE},
    {"SUB E", 0, FALSE},
    {"SUB H", 0, FALSE},
    {"SUB L", 0, FALSE},
    {"SUB (HL)", 0, FALSE},
    {"SUB A", 0, FALSE},
    {"SBC B", 0, FALSE},
    {"SBC C", 0, FALSE},
    {"SBC D", 0, FALSE},
    {"SBC E", 0, FALSE},
    {"SBC H", 0, FALSE},
    {"SBC L", 0, FALSE},
    {"SBC (HL)", 0, FALSE},
    {"SBC A", 0, FALSE},
    
    {"AND B", 0, FALSE},
    {"AND C", 0, FALSE},
    {"AND D", 0, FALSE},
    {"AND E", 0, FALSE},
    {"AND H", 0, FALSE},
    {"AND L", 0, FALSE},
    {"AND (HL)", 0, FALSE},
    {"AND A", 0, FALSE},
    {"XOR B", 0, FALSE},
    {"XOR C", 0, FALSE},
    {"XOR D", 0, FALSE},
    {"XOR E", 0, FALSE},
    {"XOR H", 0, FALSE},
    {"XOR L", 0, FALSE},
    {"XOR (HL)", 0, FALSE},
    {"XOR A", 0, FALSE},
    
    {"OR B", 0, FALSE},
    {"OR C", 0, FALSE},
    {"OR D", 0, FALSE},
    {"OR E", 0, FALSE},
    {"OR H", 0, FALSE},
    {"OR L", 0, FALSE},
    {"OR (HL)", 0, FALSE},
    {"OR A", 0, FALSE},
    {"CP B", 0, FALSE},
    {"CP C", 0, FALSE},
    {"CP D", 0, FALSE},
    {"CP E", 0, FALSE},
    {"CP H", 0, FALSE},
    {"CP L", 0, FALSE},
    {"CP (HL)", 0, FALSE},
    {"CP A", 0, FALSE},
    
    {"RET NZ", 0, FALSE},
    {"POP BC", 0, FALSE},
    {"JP NZ, %04Xh", 2, FALSE},
    {"JP %04Xh", 2, FALSE},
    {"CALL NZ, %04Xh", 2, FALSE},
    {"PUSH BC", 0, FALSE},
    {"ADD %02Xh", 1, FALSE},
    {"RST 00h", 0, FALSE},
    {"RET Z", 0, FALSE},
    {"RET", 0, FALSE},
    {"JP Z, %04Xh", 2, FALSE},
    {"PREFIX CB", 1, FALSE},
    {"CALL Z, %04Xh", 2, FALSE},
    {"CALL %04Xh", 2, FALSE},
    {"ADC %02Xh", 1, FALSE},
    {"RST 08h", 0, FALSE},
    
    {"RET NC", 0, FALSE},
    {"POP DE", 0, FALSE},
    {"JP NC, %04Xh", 2, FALSE},
    {"!error (D3h)", 0, FALSE},
    {"CALL NC, %04Xh", 2, FALSE},
    {"PUSH DE", 0, FALSE},
    {"SUB %02Xh", 1, FALSE},
    {"RST 10h", 0, FALSE},
    {"RET C", 0, FALSE},
    {"RETI", 0, FALSE},
    {"JP C, %04Xh", 2, FALSE},
    {"!error (DBh)", 0, FALSE},
    {"CALL C, %04Xh", 2, FALSE},
    {"!error (DDh)", 0, FALSE},
    {"SBC %02Xh", 1, FALSE},
    {"RST 18h", 0, FALSE},
    
    {"LDH (0xFF%02X), A", 1, FALSE},
    {"POP HL", 0, FALSE},
    {"LDH (C), A", 0, FALSE},
    {"!error (E3h)", 0, FALSE},
    {"!error (E4h)", 0, FALSE},
    {"PUSH HL", 0, FALSE},
    {"AND %02Xh", 1, FALSE},
    {"RST 20h", 0, FALSE},
    {"ADD SP, %d", 1, FALSE},
    {"JP (HL)", 0, FALSE},
    {"LD (%04Xh), A", 2, FALSE},
    {"!error (EBh)", 0, FALSE},
    {"!error (ECh)", 0, FALSE},
    {"!error (EDh)", 0, FALSE},
    {"XOR %02Xh", 1, FALSE},
    {"RST 28h", 0, FALSE},
    
    {"LDH A, (0xFF%02X)", 1, FALSE},
    {"POP AF", 0, FALSE},
    {"LDH A, (C)", 0, FALSE},
    {"DI", 0, FALSE},
    {"!error (F4h)", 0, FALSE},
    {"PUSH AF", 0, FALSE},
    {"OR %02Xh", 1, FALSE},
    {"RST 30h", 0, FALSE},
    {"LD HL,SP+%d", 1, FALSE},
    {"LD SP, HL", 0, FALSE},
    {"LD A, (%04Xh)", 2, FALSE},
    {"EI", 0, FALSE},
    {"!error (ECh)", 0, FALSE},
    {"!error (EDh)", 0, FALSE},
    {"CP %02Xh", 1, FALSE},
    {"RST 38h", 0, FALSE},
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

        if (instruction->relativeAddress)
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