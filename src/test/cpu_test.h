
#include "../cpu.h"
#include "../instructions.h"
#include "../../memory.h"

#define LOAD_PROGRAM(dest, src) memCopy(dest, src, sizeof(src));

int testCPUState(
    char *testName,
    char *testOutput,
    struct CPUState* actual,
    struct CPUState* expected
);

int testInt(
    char *testName,
    char *testOutput,
    int actual,
    int expected
);

int testRST(struct CPUState* cpu, struct Memory* memory, char* testOutput, int instruction, int targetAddress);

unsigned char* getRegisterPointer(struct CPUState* cpu, unsigned char* hlTarget, unsigned char* d8Target, int registerIndex);

int testSingleADD(
    struct CPUState* cpu, 
    struct Memory* memory,
    char* testOutput,
    int srcRegister,
    int baseInstruction,
    int cFlag
);

int testSingleBitwise(
    struct CPUState* cpu, 
    struct Memory* memory,
    char* testOutput,
    int srcRegister,
    int baseInstruction
);

int run0x0Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0x1Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0x2Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0x3Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0x4_7Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0x8_9Tests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0xA_BTests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int runPrefixCBTests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0xCTests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0xDTests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0xETests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int run0xFTests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int runRegisterTests(struct CPUState* cpu, struct Memory* memory, char* testOutput);
int runInterruptTests(struct CPUState* cpu, struct Memory* memory, char* testOutput);

extern char* registerNames[];
extern int registerOffset[];

int runCPUTests(char* testOutput);

#define REGISTER_COUNT 8
#define B_REGISTER_INDEX 0
#define C_REGISTER_INDEX 1
#define D_REGISTER_INDEX 2
#define E_REGISTER_INDEX 3
#define H_REGISTER_INDEX 4
#define L_REGISTER_INDEX 5
#define HL_REGISTER_INDEX 6
#define A_REGISTER_INDEX 7
#define d8_REGISTER_INDEX 8
