
#ifndef __HEAP_H

struct HeapNode {
    unsigned short key;
    unsigned short id;
    unsigned short heapNodeCount;
};

#define NEXT_HEAP_INDEX(currNode, isRight) (((currNode) << 1) + ((isRight) ? 2 : 1))

int heapNodeCount(struct HeapNode* heap);
void heapInit(struct HeapNode* heap, int capacity);
void heapAdd(struct HeapNode* heap, struct HeapNode* value, int currentIndex);
void heapTake(struct HeapNode* heap, struct HeapNode* result, int currentIndex);
int heapValidate(struct HeapNode* heap, int currentIndex);

#endif