
#include "compress.h"
#include "heap.h"

#define MAX_HUFFMAN_NODES   512
#define MAX_BACK_REFERENCES 256
#define MAX_WINDOW_SIZE     1024

/**
 * Organized as a heap
 * 
 *                0 (index)
 *               / \
 *              /   \ 
 *             /     \
 *            /       \
 *           1         2
 *          /  \      /  \ 
 *         3    4    5     6
 *        / \  / \  / \   / \
 *       7  8 9 10 11 12 13 14
 *  
 */
struct HuffmanNode gHuffmanNodes[MAX_HUFFMAN_NODES];
struct HeapNode gHeapNodes[MAX_HUFFMAN_NODES];

struct HuffmanCode gHuffmanCodes[MAX_HUFFMAN_CODE_COUNT];
int gOccurence[MAX_HUFFMAN_CODE_COUNT];

struct BackReference gBackreferences[MAX_BACK_REFERENCES];
int gBackreferenceCount;

void extractCodes(struct HuffmanNode* root, struct HuffmanCode* codes, struct HuffmanCode currentCode) {
    if (root->value != NO_HUFFMAN_VALUE) {
        codes[root->value] = currentCode;
    } else {
        currentCode.code <<= 1;
        ++currentCode.bitCount;

        extractCodes(&gHuffmanNodes[root->left], codes, currentCode);
        ++currentCode.code;
        extractCodes(&gHuffmanNodes[root->right], codes, currentCode);
    }
}

struct HuffmanNode* buildHuffmanTree(int* occurenceTable) {
    int currentNodeIndex = 0;

    heapInit(gHeapNodes, MAX_HUFFMAN_NODES);

    int i;
    for (i = 0; i <= NODE_VALUE_REFERENCE; ++i) {
        if (occurenceTable[i]) {
            struct HeapNode heapNode;
            heapNode.id = currentNodeIndex;
            heapNode.key = occurenceTable[i];
            heapAdd(gHeapNodes, &heapNode, 0);

            gHuffmanNodes[currentNodeIndex].left = NO_HUFFMAN_NODE;
            gHuffmanNodes[currentNodeIndex].right = NO_HUFFMAN_NODE;
            gHuffmanNodes[currentNodeIndex].value = i;
            ++currentNodeIndex;
        }
    }


    while (heapValidate(gHeapNodes, 0) && heapNodeCount(gHeapNodes) > 1) {
        struct HeapNode left;
        struct HeapNode right;
        heapTake(gHeapNodes, &left, 0);
        heapTake(gHeapNodes, &right, 0);

        struct HeapNode nextNode;
        nextNode.id = currentNodeIndex;
        nextNode.key = left.key + right.key;
        heapAdd(gHeapNodes, &nextNode, 0);

        gHuffmanNodes[currentNodeIndex].left = left.id;
        gHuffmanNodes[currentNodeIndex].right = right.id;
        gHuffmanNodes[currentNodeIndex].value = NO_HUFFMAN_VALUE;
        ++currentNodeIndex;
    }

    return &gHuffmanNodes[currentNodeIndex-1];
}

int allocateHuffmanNode(int* huffmanCodeCount) {
    struct HuffmanNode* currentNode = &gHuffmanNodes[*huffmanCodeCount];
    currentNode->left = NO_HUFFMAN_NODE;
    currentNode->right = NO_HUFFMAN_NODE;
    currentNode->value = NO_HUFFMAN_VALUE;
    int result = *huffmanCodeCount;
    ++*huffmanCodeCount;
    return result;
}

void insertHuffmanNode(struct HuffmanNode* currentNode, int* huffmanCodeCount, struct HuffmanCode code, unsigned short value) {
    if (!currentNode) {
        currentNode = &gHuffmanNodes[*huffmanCodeCount];
        currentNode->left = NO_HUFFMAN_NODE;
        currentNode->right = NO_HUFFMAN_NODE;
        currentNode->value = NO_HUFFMAN_VALUE;
        ++*huffmanCodeCount;
    }

    if (code.bitCount == 0) {
        currentNode->value = value;
    } else {
        int goRight = (1 << (code.bitCount - 1)) & code.code;

        --code.bitCount;

        if (goRight) {
            if (currentNode->right == NO_HUFFMAN_VALUE) {
                currentNode->right = allocateHuffmanNode(huffmanCodeCount);
            }
            insertHuffmanNode(&gHuffmanNodes[currentNode->right], huffmanCodeCount, code, value);
        } else {
            if (currentNode->left == NO_HUFFMAN_VALUE) {
                currentNode->left = allocateHuffmanNode(huffmanCodeCount);
            }
            insertHuffmanNode(&gHuffmanNodes[currentNode->left], huffmanCodeCount, code, value);
        }
    }

}

void decodeChunk(char* output, int maxOutputLength, BitStreamReader input, void* inputData) {
    int huffmanCodeCount = 0;

    // allocate root node
    allocateHuffmanNode(&huffmanCodeCount);

    int bitCount = input(inputData, 4);

    while (bitCount) {
        struct HuffmanCode codeToAdd;
        codeToAdd.bitCount = bitCount;
        codeToAdd.code = input(inputData, bitCount);
        insertHuffmanNode(&gHuffmanNodes[0], &huffmanCodeCount, codeToAdd, input(inputData, 5));

        bitCount = input(inputData, 4);
    }

    int currentOut = 0;

    while (currentOut < maxOutputLength) {
        struct HuffmanNode* current = &gHuffmanNodes[0];

        while (current->value != NO_HUFFMAN_VALUE) {
            if (input(inputData, 1)) {
                current = &gHuffmanNodes[current->right];
            } else {
                current = &gHuffmanNodes[current->left];
            }
        }

        if (current->value == NODE_VALUE_END_STREAM) {
            break;
        } else if (current->value == NODE_VALUE_REFERENCE) {
            int backReference = input(inputData, BACK_REF_OFFSET_BITS) + MIN_BACK_REF_DIST;
            int length = input(inputData, BACK_REF_LENGTH_BITS) + MIN_BACK_REF_LEN;

            int i;

            for (i = 0; i < length && currentOut + i < maxOutputLength; ++i) {
                output[currentOut + i] = output[currentOut + i - backReference];
            }
        } else {
            output[currentOut] = (char)current->value;
            ++currentOut;
        }
    }
}

int measureMatchLength(char* source, int a, int b, int sourceLength) {
    int result = 0;

    while (source[a + result] == source[b + result] &&
        a + result < sourceLength &&
        result < BACK_REF_MAX_LENGTH) {
        ++result;
    }

    return result;
}

int searchForBackReference(char* source, int at, int sourceLen, struct BackReference* result) {
    // if (at > 0 && source[at] == source[at-1]) {
    //     int matchLength = measureMatchLength(source, at, at-1, sourceLen);

    //     if (matchLength >= MIN_BACK_REF_LEN) {
    //         result->addr = at-1;
    //         result->length = matchLength;
    //         return 1;
    //     }
    // }

    return 0;
}

void encodeChunk(char* source, int length, BitStreamWriter output, void* outputData) {
    int i;

    for (i = 0; i < MAX_HUFFMAN_CODE_COUNT; ++i) {
        gOccurence[i] = 0;
    }

    gBackreferenceCount = 0;

    gOccurence[NODE_VALUE_END_STREAM] = 1;

    for (i = 0; i < length; ++i) {
        struct BackReference backRef;
        if (gBackreferenceCount < MAX_BACK_REFERENCES && 
            searchForBackReference(source, i, length, &backRef)) {
            ++gOccurence[NODE_VALUE_REFERENCE];
            gBackreferences[gBackreferenceCount] = backRef;
            ++gBackreferenceCount;
        } else {
            ++gOccurence[source[i]];
        }
    }

    struct HuffmanNode* rootHuffman = buildHuffmanTree(gOccurence);
    struct HuffmanCode blankCode;
    blankCode.code = 0;
    blankCode.bitCount = 0;
    extractCodes(rootHuffman, gHuffmanCodes, blankCode);

    for (i = 0; i < MAX_HUFFMAN_CODE_COUNT; ++i) {
        if (gOccurence[i]) {
            output(outputData, gHuffmanCodes[i].bitCount, 4);
                output(outputData, gHuffmanCodes[i].code, gHuffmanCodes[i].bitCount);
            output(outputData, i, 5);
        }
    }

    // null terminator
    output(outputData, 0, 4);

    int nextBackref = 0;

    for (i = 0; i < length;) {
        if (nextBackref < MAX_BACK_REFERENCES &&
            gBackreferences[nextBackref].addr == i) {
            output(outputData, gHuffmanCodes[NODE_VALUE_REFERENCE].code, gHuffmanCodes[NODE_VALUE_REFERENCE].bitCount);
            output(outputData, gBackreferences[nextBackref].distnace - MIN_BACK_REF_DIST, BACK_REF_OFFSET_BITS);
            output(outputData, gBackreferences[nextBackref].length - MIN_BACK_REF_LEN, BACK_REF_LENGTH_BITS);

            i += gBackreferences[nextBackref].length;
            ++nextBackref;
        } else {
            output(outputData, gHuffmanCodes[source[i]].code, gHuffmanCodes[source[i]].bitCount);
            ++i;
        }
    }

    output(outputData, gHuffmanCodes[NODE_VALUE_END_STREAM].code, gHuffmanCodes[NODE_VALUE_END_STREAM].bitCount);
}