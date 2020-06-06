
#ifndef _AUDIO_H
#define _AUDIO_H

#define AUDIO_BUFFER_COUNT 4

struct Memory;

struct SquareWavePattern {
    unsigned char sweep;
    unsigned char length_pattern;
    unsigned char volume;
    unsigned char frequencyLo;
    unsigned char frequencyHi;
};

struct AudioSample
{
    short l;
    short r;
};

struct AudioState
{
    struct AudioSample* buffers[AUDIO_BUFFER_COUNT];
    unsigned short frequency;
    unsigned short samplesPerBuffer;
    unsigned short currentBuffer;
    unsigned short currentBufferIndex;
    unsigned short sound1Cycle;
    unsigned short sound2Cycle;
    unsigned short sound3Cycle;
    unsigned short sound4Cycle;
    int apuTicks;
};

void initAudio(struct AudioState* audioState, int frequency, int frameRate);
void updateAudio(struct Memory* memoryMap, int apuTicks);

#endif