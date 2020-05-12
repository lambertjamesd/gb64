
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