
#ifndef _AUDIO_H
#define _AUDIO_H
#include <ultra64.h>

#define AUDIO_BUFFER_COUNT 4

struct Memory;

#define GET_WAVE_DUTY(length_pattern) (((length_pattern) & 0xC0) >> 6)
#define GET_SQUARE_VOLUME(volume) (((volume) >> 4) & 0xF)
#define GET_SOUND_FREQ(frequencyHi, frequencyLo) ((((int)(frequencyHi) & 0x7) << 8) | (int)(frequencyLo))
#define GET_PCM_VOLUME(volume) (((volume) >> 5) & 0x3)

#define NOISE_MAX_CLOCK_SHIFT 13

/*
steps_cycle = 0x10000 steps/cycle - or the number of steps to overflow the cycleProgress
freq = (0x20000 / (2048 - x)) cycles/second - how to calculate the frequency from the gb sound register 
sampleRate = samples/second - the number of samples per second the output is expecting
? steps/sample = steps_cycle * freq / sampleRate
*/
#define CYCLE_STEP(frequency, sampleRate) (int)(0x200000000L / ((0x800L - (frequency)) * (sampleRate)))

struct AudioSample
{
    short l;
    short r;
};

struct SquareWaveSound {
    u16 cycle;
    u16 waveDuty;
    u16 volume;
    u16 frequency;
    u16 length;
    u16 unused;
};

struct PCMSound {
    u16 cycle;
    u16 volume;
    u16 frequency;
    u16 length;
    u8 pcm[0x10];
};

enum LFSRWidth {
    LFSRWidth15,
    LFSRWidth7
};

struct NoiseSound {
    u16 volume;
    u16 length;
    u16 lfsr;
    // fixed point numbers 8:24
    u32 accumulator;
    u32 sampleStep;
    enum LFSRWidth lfsrWidth;
};

struct AudioState
{
    struct AudioSample* buffers[AUDIO_BUFFER_COUNT];
    struct SquareWaveSound sound1;
    struct SquareWaveSound sound2;
    struct PCMSound pcmSound;
    struct NoiseSound noiseSound;
    int apuTicks;
    u16 sampleRate;
    u16 samplesPerBuffer;
    u16 currentWriteBuffer;
    u16 currentSampleIndex;
    u16 nextPlayBuffer;
};

void initAudio(struct AudioState* audioState, int sampleRate, int frameRate);
void updateAudio(struct Memory* memoryMap, int apuTicks);

#endif