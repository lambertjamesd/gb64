
#include "stream.h"

void initBitFromByteStreamReader(struct BitFromByteStreamReader* reader, ByteStreamReader source, void* sourceData) {
    reader->source = source;
    reader->sourceData = sourceData;
    reader->currentByte = 0;
    reader->bitsLeft = 0;
}

unsigned int bitFromByteStreamReader(void* data, int bitCount) {
    struct BitFromByteStreamReader* asReader = (struct BitFromByteStreamReader*)data;

    unsigned int result = 0;

    while (bitCount > 0) {
        if (asReader->bitsLeft == 0) {
            asReader->source(asReader->sourceData, &asReader->currentByte, 1);
            asReader->bitsLeft = 8;
        }

        int bitsToRead = 0;

        if (bitCount <= asReader->bitsLeft) {
            bitsToRead = bitCount;
        } else {
            bitsToRead = asReader->bitsLeft;
        }

        int bitChunk = (asReader->currentByte >> (8 - bitsToRead)) & ((1 << bitsToRead) - 1);
        result = (result << bitsToRead) | bitChunk;
        asReader->bitsLeft -= bitsToRead;
        asReader->currentByte <<= bitsToRead;
        bitsToRead -= bitsToRead;
    }

    return result;
}

void initBitFromByteStreamWriter(struct BitFromByteStreamWriter* writer, BytetreamWriter target, void* targetData) {
    writer->target = target;
    writer->targetData = targetData;
    writer->currentByte = 0;
    writer->currentBitCount = 0;
}

void bitFromByteStreamWriter(void* data, unsigned int value, int bitCount) {
    struct BitFromByteStreamWriter* asWriter = (struct BitFromByteStreamWriter*)data;

    value <<= (32 - bitCount);

    while (bitCount > 0) {
        int bitsToWrite = 0;
        int spaceLeft = 8 - asWriter->currentBitCount;

        if (bitCount < spaceLeft) {
            bitsToWrite = bitCount;
        } else {
            bitsToWrite = spaceLeft;
        }

        int bitChunk = value >> (32 - bitsToWrite);
        asWriter->currentByte = (asWriter->currentByte << bitsToWrite) | bitChunk;
        asWriter->currentBitCount -= bitsToWrite;
        bitCount -= bitsToWrite;

        if (asWriter->currentBitCount == 8) {
            asWriter->target(asWriter->targetData, &asWriter->currentByte, 1);
            asWriter->currentByte = 0;
            asWriter->currentBitCount = 0;
        }
    }
}

void bitFromByteFlush(struct BitFromByteStreamWriter* writer) {
    if (writer->currentBitCount != 0) {
        bitFromByteStreamWriter(writer, 0, 8 - writer->currentBitCount);
    }
}