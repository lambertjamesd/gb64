
#ifndef _STREAM_H
#define _STREAM_H

typedef void (*ByteStreamReader)(void* data, char* value, int byteCount);
typedef void (*BytetreamWriter)(void* data, char *value, int byteCount);

typedef unsigned int (*BitStreamReader)(void* data, int bitCount);
typedef void (*BitStreamWriter)(void* data, unsigned int value, int bitCount);

struct BitFromByteStreamReader {
    ByteStreamReader source;
    void* sourceData;
    unsigned char currentByte;
    unsigned char bitsLeft;
};

void initBitFromByteStreamReader(struct BitFromByteStreamReader* reader, ByteStreamReader source, void* sourceData);
unsigned int bitFromByteStreamReader(void* data, int bitCount);

struct BitFromByteStreamWriter {
    BytetreamWriter target;
    void* targetData;
    unsigned char currentByte;
    unsigned char currentBitCount;
};

void initBitFromByteStreamWriter(struct BitFromByteStreamWriter* writer, BytetreamWriter target, void* targetData);
void bitFromByteStreamWriter(void* data, unsigned int value, int bitCount);
void bitFromByteFlush(struct BitFromByteStreamWriter* writer);

#endif