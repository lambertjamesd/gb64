
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