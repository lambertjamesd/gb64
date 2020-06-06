
#ifndef _AUDIO_H
#define _AUDIO_H

struct AudioState
{
    short** buffers;
    int bufferCount;
    int bufferSize;
    int frequency;
};

void initAudio(struct AudioState* audioState, int frequency, int frameRate);

#endif