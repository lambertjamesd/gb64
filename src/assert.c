
#include "assert.h"

#if DEBUG

asm(
".global teqassert\n"
".balign 4\n"
"teqassert:\n"
    "teq $a0, $0\n"
    "jr $ra\n"
    "nop\n"
);

#endif