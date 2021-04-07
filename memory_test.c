

#ifndef DEBUG
#define DEBUG 1
#endif

#include "src/assert.h"
#include "memory.h"

extern struct HeapSegment* gFirstFreeSegment;

void test_malloc()
{
    struct HeapSegment* startSegAddr = gFirstFreeSegment;
    struct HeapSegment startSeg = *gFirstFreeSegment;

    void* a = malloc(16);
    void* b = malloc(16);
    void* c = malloc(16);

    free(b);
    c = realloc(c, 32);

    free(a);
    free(b);
    free(c);

    b = malloc(16);
    
    assert(a == b);
    free(b);

    assert(startSegAddr == gFirstFreeSegment);
    assert(startSeg.segmentEnd == gFirstFreeSegment->segmentEnd);
}