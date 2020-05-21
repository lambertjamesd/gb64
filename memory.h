
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