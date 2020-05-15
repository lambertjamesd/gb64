
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

int run0x0Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0x1Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0x2Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);
int run0x3Tests(struct Z80State* z80, void** memoryMap, unsigned char* memory, char* testOutput);

int runTests(char* testOutput);

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