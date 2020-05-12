
struct Z80State {
    unsigned char a;
    unsigned char f;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char h;
    unsigned char l;
    unsigned short sp;
    unsigned short pc;
    int state;
};

extern int runZ80CPU(struct Z80State* state, void** memoryMap, int cyclesToRun);

#define GB_MEMORY_MAP_SIZE  0x10000
extern struct Z80State gZ80;
extern unsigned char gbMemory[GB_MEMORY_MAP_SIZE];

extern void zeroMemory(unsigned char* memory, int size);
extern void initializeZ80(struct Z80State* state);