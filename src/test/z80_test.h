
#include "../z80.h"

int testZ80State(
    char *testName,
    char *testOutput,
    struct Z80State* actual,
    struct Z80State* expected
);

int testInt(
    char *testName,
    char *testOutput,
    int actual,
    int expected
);

int testRST(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput, int instruction, int targetAddress);

unsigned char* getRegisterPointer(struct Z80State* z80, unsigned char* hlTarget, unsigned char* d8Target, int registerIndex);

int testSingleADD(
    struct Z80State* z80, 
    void** memoryMap, 
    unsigned char* memory, 
    char* testOutput,
    int srcRegister,
    int baseInstruction,
    int cFlag
);

int run0x0Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0x1Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0x2Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0x3Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0x4_7Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0x8_9Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0xA_BTests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0xCTests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0xDTests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0xETests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);

extern char* registerNames[];
extern int registerOffset[];

int runTests(char* testOutput);

#define REGISTER_COUNT 8
#define HL_REGISTER_INDEX 6
#define A_REGISTER_INDEX 7
#define d8_REGISTER_INDEX 8

#define Z80_NOP         0x00
#define Z80_LD_BC_d16   0x01
#define Z80_LD_BC_A     0x02
#define Z80_INC_BC      0x03
#define Z80_INC_B       0x04
#define Z80_DEC_B       0x05
#define Z80_LD_B_d8     0x06
#define Z80_RLCA        0x07
#define Z80_LD_a16_SP   0x08
#define Z80_ADD_HL_BC   0x09
#define Z80_LD_A_BC     0x0A
#define Z80_DEC_BC      0x0B
#define Z80_INC_C       0x0C
#define Z80_DEC_C       0x0D
#define Z80_LD_C_d8     0x0E
#define Z80_RRCA        0x0F

#define Z80_STOP        0x10
#define Z80_LD_DE_d16   0x11
#define Z80_LD_DE_A     0x12
#define Z80_INC_DE      0x13
#define Z80_INC_D       0x14
#define Z80_DEC_D       0x15
#define Z80_LD_D_d8     0x16
#define Z80_RLA         0x17
#define Z80_JR          0x18
#define Z80_ADD_HL_DE   0x19
#define Z80_LD_A_DE     0x1A
#define Z80_DEC_DE      0x1B
#define Z80_INC_E       0x1C
#define Z80_DEC_E       0x1D
#define Z80_LD_E_d8     0x1E
#define Z80_RRA         0x1F

#define Z80_JR_NZ       0x20
#define Z80_LD_HL_d16   0x21
#define Z80_LDI_HL_A    0x22
#define Z80_INC_HL      0x23
#define Z80_INC_H       0x24
#define Z80_DEC_H       0x25
#define Z80_LD_H_d8     0x26
#define Z80_DAA         0x27
#define Z80_JR_Z        0x28
#define Z80_ADD_HL_HL   0x29
#define Z80_LDI_A_HL    0x2A
#define Z80_DEC_HL      0x2B
#define Z80_INC_L       0x2C
#define Z80_DEC_L       0x2D
#define Z80_LD_L_d8     0x2E
#define Z80_CPL         0x2F

#define Z80_JR_NC       0x30
#define Z80_LD_SP_d16   0x31
#define Z80_LDD_HL_A    0x32
#define Z80_INC_SP      0x33
#define Z80_INC_HL_ADDR 0x34
#define Z80_DEC_HL_ADDR 0x35
#define Z80_LD_HL_d8    0x36
#define Z80_SCF         0x37
#define Z80_JR_C        0x38
#define Z80_ADD_HL_SP   0x39
#define Z80_LDD_A_HL    0x3A
#define Z80_DEC_SP      0x3B
#define Z80_INC_A       0x3C
#define Z80_DEC_A       0x3D
#define Z80_LD_A_d8     0x3E
#define Z80_CCF         0x3F

#define Z80_LD_B_B      0x40
#define Z80_LD_B_C      0x41
#define Z80_LD_B_D      0x42
#define Z80_LD_B_E      0x43
#define Z80_LD_B_H      0x44
#define Z80_LD_B_L      0x45
#define Z80_LD_B_HL     0x46
#define Z80_LD_B_A      0x47
#define Z80_LD_C_B      0x48
#define Z80_LD_C_C      0x49
#define Z80_LD_C_D      0x4A
#define Z80_LD_C_E      0x4B
#define Z80_LD_C_H      0x4C
#define Z80_LD_C_L      0x4D
#define Z80_LD_C_HL     0x4E
#define Z80_LD_C_A      0x4F

#define Z80_LD_D_B      0x50
#define Z80_LD_D_C      0x51
#define Z80_LD_D_D      0x52
#define Z80_LD_D_E      0x53
#define Z80_LD_D_H      0x54
#define Z80_LD_D_L      0x55
#define Z80_LD_D_HL     0x56
#define Z80_LD_D_A      0x57
#define Z80_LD_E_B      0x58
#define Z80_LD_E_C      0x59
#define Z80_LD_E_D      0x5A
#define Z80_LD_E_E      0x5B
#define Z80_LD_E_H      0x5C
#define Z80_LD_E_L      0x5D
#define Z80_LD_E_HL     0x5E
#define Z80_LD_E_A      0x5F

#define Z80_LD_H_B      0x60
#define Z80_LD_H_C      0x61
#define Z80_LD_H_D      0x62
#define Z80_LD_H_E      0x63
#define Z80_LD_H_H      0x64
#define Z80_LD_H_L      0x65
#define Z80_LD_H_HL     0x66
#define Z80_LD_H_A      0x67
#define Z80_LD_L_B      0x68
#define Z80_LD_L_C      0x69
#define Z80_LD_L_D      0x6A
#define Z80_LD_L_E      0x6B
#define Z80_LD_L_H      0x6C
#define Z80_LD_L_L      0x6D
#define Z80_LD_L_HL     0x6E
#define Z80_LD_L_A      0x6F

#define Z80_LD_HL_B     0x70
#define Z80_LD_HL_C     0x71
#define Z80_LD_HL_D     0x72
#define Z80_LD_HL_E     0x73
#define Z80_LD_HL_H     0x74
#define Z80_LD_HL_L     0x75
#define Z80_HALT        0x76
#define Z80_LD_HL_A     0x77
#define Z80_LD_A_B      0x78
#define Z80_LD_A_C      0x79
#define Z80_LD_A_D      0x7A
#define Z80_LD_A_E      0x7B
#define Z80_LD_A_H      0x7C
#define Z80_LD_A_L      0x7D
#define Z80_LD_A_HL     0x7E
#define Z80_LD_A_A      0x7F

#define Z80_ADD_A_B     0x80
#define Z80_ADC_A_B     0x88

#define Z80_SUB_A_B     0x90
#define Z80_SBC_A_B     0x98

#define Z80_AND_A_B     0xA0
#define Z80_XOR_A_B     0xA8

#define Z80_OR_A_B      0xB0
#define Z80_CP_A_B      0xB8

#define Z80_RET_NZ      0xC0
#define Z80_POP_BC      0xC1
#define Z80_JP_NZ_a16   0xC2
#define Z80_JP_a16      0xC3
#define Z80_CALL_NZ     0xC4
#define Z80_PUSH_BC     0xC5
#define Z80_ADD_A_d8    0xC6
#define Z80_RST_00H     0xC7
#define Z80_RET_Z       0xC8
#define Z80_RET         0xC9
#define Z80_JP_Z_a16    0xCA
#define Z80_PREFIX_CB   0xCB
#define Z80_CALL_Z      0xCC
#define Z80_CALL        0xCD
#define Z80_ADC_A_d8    0xCE
#define Z80_RST_08H     0xCF

#define Z80_RET_NC      0xD0
#define Z80_POP_DE      0xD1
#define Z80_JP_NC_a16   0xD2
#define Z80_CALL_NC     0xD4
#define Z80_PUSH_DE     0xD5
#define Z80_SUB_A_d8    0xD6
#define Z80_RST_10H     0xD7
#define Z80_RET_C       0xD8
#define Z80_RETI        0xD9
#define Z80_JP_C_a16    0xDA
#define Z80_CALL_C      0xDC
#define Z80_SBC_A_d8    0xDE
#define Z80_RST_18H     0xDF

#define Z80_LDH_a8_A    0xE0
#define Z80_POP_HL      0xE1
#define Z80_LD_C_ADDR_A 0xE2
#define Z80_PUSH_HL     0xE5
#define Z80_AND_A_d8    0xE6
#define Z80_RST_20H     0xE7
#define Z80_ADD_SP_r8   0xE8
#define Z80_JP_HL       0xE9
#define Z80_a16_A       0xEA
#define Z80_XOR_A_d8    0xEE
#define Z80_RST_28H     0xEF

#define Z80_LDH_A_a8    0xF0
#define Z80_POP_AF      0xF1
#define Z80_LD_A_C_ADDR 0xF2
#define Z80_DI          0xF3
#define Z80_PUSH_AF     0xF5
#define Z80_OR_A_d8     0xF6
#define Z80_RST_30H     0xF7
#define Z80_HL_SP_r8    0xF8
#define Z80_SP_HL       0xF9
#define Z80_A_a16       0xFA
#define Z80_EI          0xFB
#define Z80_CP_A_d8     0xFE
#define Z80_RST_38H     0xFF