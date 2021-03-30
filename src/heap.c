
#include "heap.h"

void heapInit(struct HeapNode* heap, int capacity) {
    int i;

    for (i = 0; i < capacity; ++i) {
        heap[i].heapNodeCount = 0;
    }
}


int heapNodeCount(struct HeapNode* heap) {
    return heap[0].heapNodeCount;
}

void heapAdd(struct HeapNode* heap, struct HeapNode* value, int currentIndex) {
    if (heap[currentIndex].heapNodeCount == 0) {
        heap[currentIndex].key = value->key;
        heap[currentIndex].id = value->id;
    } else {
        int leftIndex = NEXT_HEAP_INDEX(currentIndex, 0);
        int rightIndex = NEXT_HEAP_INDEX(currentIndex, 1);

        int nextIndex = (heap[leftIndex].heapNodeCount < heap[rightIndex].heapNodeCount) ? 
            leftIndex :
            rightIndex;

        if (value->key < heap[currentIndex].key) {
            heapAdd(heap, &heap[currentIndex], nextIndex);
            heap[currentIndex].key = value->key;
            heap[currentIndex].id = value->id;
        } else {
            heapAdd(heap, value, nextIndex);
        }
    }

    ++heap[currentIndex].heapNodeCount;
}

void heapTake(struct HeapNode* heap, struct HeapNode* result, int currentIndex) {
    --heap[currentIndex].heapNodeCount;
    *result = heap[currentIndex];

    if (heap[currentIndex].heapNodeCount != 0) {
        int leftIndex = NEXT_HEAP_INDEX(currentIndex, 0);
        int rightIndex = NEXT_HEAP_INDEX(currentIndex, 1);

        heapTake(
            heap,
            &heap[currentIndex], 
            (heap[leftIndex].key < heap[rightIndex].key) ? 
                leftIndex :
                rightIndex
        );
    }
}