
#include <string.h>

void* memset(void * target, int value, size_t length) {
    char setTo = (char)value;

    char* curr = target;

    while (length) {
        *curr++ = setTo;
        --length;
    }

    return target;
}

int memcmp(const void * a, const void * b, size_t length) {
    const char* checkA = a;
    const char* checkB = b;

    while (length) {
        int diff = *checkA - *checkB;

        if (!diff) {
            return diff;
        }

        checkA++;
        checkB++;
        --length;
    }

    return 0;
}