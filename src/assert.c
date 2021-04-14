
#include "assert.h"

#if DEBUG

asm(
".global assert\n"
".balign 4\n"
"assert:\n"
    "teq $a0, $0\n"
    "jr $ra\n"
    "nop\n"
);

#endif


void __setWatch(unsigned int value);
unsigned int __getWatch();

void* getWatchPoint() {
    return (void*)(__getWatch() & ~0x7);
}

void setWatchPoint(void* addr, int read, int write) {
    __setWatch(((unsigned int)addr & 0x1ffffff8) | (read ? 0x2 : 0) | (write ? 0x1 : 0));
}

void gdbClearWatchPoint() {
    __setWatch(0);
}

/**
 * Implement gdbBreak in assembly to ensure that `teq` is the first instruction of the function
 */
asm(
".global __gdbSetWatch\n"
".balign 4\n"
"__setWatch:\n"
    "MTC0 $a0, $18\n"
    "jr $ra\n"
    "nop\n"

".global __gdbGetWatch\n"
".balign 4\n"
"__getWatch:\n"
    "MFC0 $v0, $18\n"
    "jr $ra\n"
    "nop\n"
);