
#include <ultra64.h>
#include "memory.h"

char* gCurrentHeapPosition;

void _checkInitHeap()
{
    if (gCurrentHeapPosition == 0)
    {
        gCurrentHeapPosition = (char*)&_gMemoryStart;
        // word align the heap
        gCurrentHeapPosition = (char*)((int)(gCurrentHeapPosition + 3) & ~0x3);
    }
}


void *cacheFreePointer(void* target)
{
    return (void*)((int)target & 0x0FFFFFFF | 0xA0000000);
}

void *malloc(unsigned int size)
{
    void *result;
    _checkInitHeap();
    // word align
    size = (size + 3) & (~0x3);
    
    if (getFreeBytes() < size)
    {
        return 0;
    }
    else
    {
        result = gCurrentHeapPosition;
        gCurrentHeapPosition += size;
        return result;
    }
}

int getFreeBytes()
{
    _checkInitHeap();
    return (osGetMemSize() - ((int)gCurrentHeapPosition & 0xFFFFFFF)) & ~3;
}