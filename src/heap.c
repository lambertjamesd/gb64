
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

int heapValidate(struct HeapNode* heap, int currentIndex) {
    struct HeapNode* currentNode = &heap[currentIndex];
    
    if (currentNode->heapNodeCount == 1 || currentNode->heapNodeCount == 0) {
        return 1;
    }

    int leftIndex = NEXT_HEAP_INDEX(currentIndex, 0);
    int rightIndex = NEXT_HEAP_INDEX(currentIndex, 1);

    struct HeapNode* leftNode = &heap[leftIndex];
    struct HeapNode* rightNode = &heap[rightIndex];

    if (currentNode->heapNodeCount != leftNode->heapNodeCount + rightNode->heapNodeCount + 1) {
        return 0;
    }

    if (leftNode->heapNodeCount != 0 && currentNode->key > leftNode->key || 
        rightNode->heapNodeCount !=0 && currentNode->key > rightNode->key) {
        return 0;
    }

    return heapValidate(heap, leftIndex) && heapValidate(heap, rightIndex);
}

void heapTake(struct HeapNode* heap, struct HeapNode* result, int currentIndex) {
    --heap[currentIndex].heapNodeCount;
    result->id = heap[currentIndex].id;
    result->key = heap[currentIndex].key;

    if (heap[currentIndex].heapNodeCount != 0) {
        int leftIndex = NEXT_HEAP_INDEX(currentIndex, 0);
        int rightIndex = NEXT_HEAP_INDEX(currentIndex, 1);

        int useIndex;

        if (heap[leftIndex].heapNodeCount == 0) {
            useIndex = rightIndex;
        } else if (heap[rightIndex].heapNodeCount == 0) {
            useIndex = leftIndex;
        } else {
            useIndex = (heap[leftIndex].key < heap[rightIndex].key) ? 
                leftIndex :
                rightIndex;
        }

        heapTake(
            heap,
            &heap[currentIndex], 
            useIndex
        );
    }
}