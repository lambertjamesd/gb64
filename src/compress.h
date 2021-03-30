
#ifndef __COMPRESS_H
#define __COMPRESS_H

#include "stream.h"

#define NODE_VALUE_END_STREAM   256
#define NODE_VALUE_REFERENCE    257
#define MAX_HUFFMAN_CODE_COUNT  258

#define NO_HUFFMAN_NODE         0xFFFF
#define NO_HUFFMAN_VALUE        0xFFFF

#define BACK_REF_LENGTH_BITS    8
#define BACK_REF_OFFSET_BITS    10

#define MIN_BACK_REF_LEN        4
#define MIN_BACK_REF_DIST       1

#define BACK_REF_MAX_LENGTH     (1 << BACK_REF_LENGTH_BITS)
#define BACK_REF_MAX_OFFSET     (1 << BACK_REF_OFFSET_BITS)

typedef unsigned int HeapIndex;

struct HuffmanNode {
    unsigned short left;
    unsigned short right;
    unsigned short value;
};

struct HuffmanCode {
    unsigned short code;
    unsigned short bitCount;
};

struct BackReference {
    unsigned short distnace;
    unsigned short length;
    int addr;
};

void encodeChunk(char* source, int length, BitStreamWriter output, void* outputData);
void decodeChunk(char* output, int maxOutputLength, BitStreamReader input, void* inputData);

#endif

