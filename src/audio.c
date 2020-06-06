#include "audio.h"
#include "memory_map.h"
#include "../memory.h"
#include <ultra64.h>

#include "debug_out.h"

#define SAMPLE_0 -0x1FFF
#define SAMPLE_1 0x1FFF

short wavePattern[4][8] = {
    {SAMPLE_0, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1},
    {SAMPLE_0, SAMPLE_0, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1},
    {SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1},
    {SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_1, SAMPLE_1},
};

void initAudio(struct AudioState* audioState, int frequency, int frameRate)
{
	audioState->frequency = osAiSetFrequency(frequency);
	// align to 8 bytes
	audioState->samplesPerBuffer = (2 * audioState->frequency / frameRate) & ~1;
	audioState->currentBuffer = 0;
	audioState->apuTicks = 0;
    audioState->currentBufferIndex = 0;
    audioState->sound1Cycle = 0;
    audioState->sound2Cycle = 0;
    audioState->sound3Cycle = 0;
    audioState->sound4Cycle = 0;

	int index;
	for (index = 0; index < AUDIO_BUFFER_COUNT; ++index)
	{
		audioState->buffers[index] = malloc(sizeof(struct AudioSample) * audioState->samplesPerBuffer);
		zeroMemory(audioState->buffers[index], sizeof(struct AudioSample) * audioState->samplesPerBuffer);
	}
}

int renderSquareWave(
    struct AudioState* state, 
    int untilSamples, 
    struct SquareWavePattern* pattern,
    unsigned short startingSoundCycle
)
{
    int sampleIndex;
    struct AudioSample* output = state->buffers[state->currentBufferIndex];

    for (sampleIndex = state->currentBufferIndex; sampleIndex < untilSamples; ++sampleIndex)
    {
        
    }
}

void updateAudio(struct Memory* memory, int apuTicks)
{
	struct AudioState* audio = &memory->audio;
	u32 playbackState = osAiGetStatus();
	u32 pendingBufferCount = 0;

	if (playbackState & AI_STATUS_FIFO_FULL)
	{
		pendingBufferCount = 2;
	}
	else if (playbackState & AI_STATUS_DMA_BUSY)
	{
		pendingBufferCount = 1;
	}
	else
	{
		pendingBufferCount = 0;
	}

	while (pendingBufferCount < 2)
	{
		int sample;
		struct AudioSample* targetBuffer = audio->buffers[audio->currentBuffer];
		int multiplier = 1300;

		for (sample = 0; sample < audio->samplesPerBuffer;)
		{
			targetBuffer[sample++].l = (audio->apuTicks * multiplier * 2) & 0xFFFF;
			targetBuffer[sample++].r = (audio->apuTicks * multiplier) & 0xFFFF;
			++audio->apuTicks;
		}

		osAiSetNextBuffer(targetBuffer, audio->samplesPerBuffer * sizeof(struct AudioSample));
		audio->currentBuffer = (audio->currentBuffer + 1) % AUDIO_BUFFER_COUNT;
		++pendingBufferCount;
        zeroMemory(audio->buffers[audio->currentBuffer], audio->samplesPerBuffer * sizeof(struct AudioSample));
	}

}