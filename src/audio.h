
#ifndef _AUDIO_H
#define _AUDIO_H

#define AUDIO_BUFFER_COUNT 4

struct Memory;

struct AudioSample
{
    short l;
    short r;
};

struct AudioState
{
    struct AudioSample* buffers[AUDIO_BUFFER_COUNT];
    int frequency;
    int samplesPerBuffer;
    int currentBuffer;
    int apuTicks;
};

void initAudio(struct AudioState* audioState, int frequency, int frameRate);
void updateAudio(struct Memory* memoryMap, int apuTicks);

#endif