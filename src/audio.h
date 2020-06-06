
#ifndef _AUDIO_H
#define _AUDIO_H

#define AUDIO_BUFFER_COUNT 4

struct Memory;

#define GET_WAVE_DUTY(length_pattern) (((length_pattern) & 0xC0) >> 6)
#define GET_VOLUME(volume) (((volume) >> 4) & 0xF)
#define GET_FREQ(frequencyHi, frequencyLo) ((((int)(frequencyHi) & 0x7) << 8) | (int)(frequencyLo))

struct AudioSample
{
    short l;
    short r;
};

struct SquareWaveSound {
    unsigned short cycle;
    unsigned short waveDuty;
    unsigned short volume;
    unsigned short frequency;
    unsigned short length;
    unsigned short unused;
};

struct AudioState
{
    struct AudioSample* buffers[AUDIO_BUFFER_COUNT];
    struct SquareWaveSound sound1;
    struct SquareWaveSound sound2;
    int apuTicks;
    unsigned short sampleRate;
    unsigned short samplesPerBuffer;
    unsigned short currentWriteBuffer;
    unsigned short currentSampleIndex;
    unsigned short nextPlayBuffer;
};

void initAudio(struct AudioState* audioState, int sampleRate, int frameRate);
void updateAudio(struct Memory* memoryMap, int apuTicks);

#endif