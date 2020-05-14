
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
#define Z80_JA          0x18