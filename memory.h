
#ifndef _MEMORY_H
#define _MEMORY_H

#define offsetof(st, m) \
    ((int)&(((st *)0)->m))

extern struct HeapSegment* gFirstHeapSegment;

struct HeapSegment
{
    struct HeapSegment* nextSegment;
    // struct HeapSegment* prevSegment;
    void* segmentEnd;
};

void initHeap();
void *cacheFreePointer(void* target);
void *malloc(unsigned int size);
void markAllocated(void* addr, int length);
int calculateBytesFree();
int calculateLargestFreeChunk();
extern void zeroMemory(void* memory, int size);

#endif