#include "audio.h"
#include "memory_map.h"
#include "../memory.h"
#include <ultra64.h>

#include "debug_out.h"

#define SAMPLE_0 -0x2000
#define SAMPLE_1 0x2000

short wavePattern[4][8] = {
    {SAMPLE_0, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1},
    {SAMPLE_0, SAMPLE_0, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1},
    {SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_1, SAMPLE_1, SAMPLE_1, SAMPLE_1},
    {SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_0, SAMPLE_1, SAMPLE_1},
};

short pcmTranslate[16] = {
	-0x2000, -0x1BBB, -0x1777, -0x1333, -0x0EEE, -0x0AAA, -0x0666, -0x0222,
	0x0222, 0x0666, 0x0AAA, 0x0EEE, 0x1333, 0x1777, 0x1BBB, 0x2000,
};

void initAudio(struct AudioState* audioState, int sampleRate, int frameRate)
{
	zeroMemory(audioState, sizeof(struct AudioState));
	audioState->sampleRate = osAiSetFrequency(sampleRate);
	// align to 8 bytes
	audioState->samplesPerBuffer = (2 * audioState->sampleRate / frameRate) & ~1;

	int index;
	for (index = 0; index < AUDIO_BUFFER_COUNT; ++index)
	{
		audioState->buffers[index] = malloc(sizeof(struct AudioSample) * audioState->samplesPerBuffer);
		zeroMemory(audioState->buffers[index], sizeof(struct AudioSample) * audioState->samplesPerBuffer);
	}
}

void renderSquareWave(
    struct AudioState* state, 
    int untilSamples, 
    struct SquareWaveSound* sound,
	int rightVolume,
	int leftVolume
)
{
    int sampleIndex;
	/*
	steps_cycle = 0x10000 steps/cycle - or the number of steps to overflow the cycleProgress
	freq = (0x20000 / (2048 - x)) cycles/second - how to calculate the frequency from the gb sound register 
	sampleRate = samples/second - the number of samples per second the output is expecting
	? steps/sample = steps_cycle * freq / sampleRate
	*/
	int cycleStep = 0x200000000L / ((0x800L - sound->frequency) * state->sampleRate);
    struct AudioSample* output = state->buffers[state->currentWriteBuffer];

    for (sampleIndex = state->currentSampleIndex; sampleIndex < untilSamples; ++sampleIndex)
    {
		// fixed width mulitply the volume
		short sample = ((int)wavePattern[sound->waveDuty][sound->cycle >> 13] * sound->volume) >> 4;
		output[sampleIndex].l += (sample * leftVolume) >> 3;
		output[sampleIndex].r += (sample * rightVolume) >> 3;
		sound->cycle += cycleStep;
    }
}

void renderPatternWave(
	struct AudioState* state,
	int untilSamples,
	struct PCMSound* sound,
	int rightVolume,
	int leftVolume
) {
    int sampleIndex;
	/*
	steps_cycle = 0x10000 steps/cycle - or the number of steps to overflow the cycleProgress
	freq = (0x20000 / (2048 - x)) cycles/second - how to calculate the frequency from the gb sound register 
	sampleRate = samples/second - the number of samples per second the output is expecting
	? steps/sample = steps_cycle * freq / sampleRate
	*/
	int cycleStep = 0x200000000L / ((0x800L - sound->frequency) * state->sampleRate);
    struct AudioSample* output = state->buffers[state->currentWriteBuffer];

    for (sampleIndex = state->currentSampleIndex; sampleIndex < untilSamples; ++sampleIndex)
    {
		int sampleIndex = sound->cycle >> 12;
		unsigned char index = sound->pcm[sampleIndex];

		short sample = pcmTranslate[((sound->cycle >> 11) & 0x1) ? index & 0xF : ((index >> 4) & 0xF)] >> (sound->volume - 1);
		output[sampleIndex].l += (sample * leftVolume) >> 3;
		output[sampleIndex].r += (sample * rightVolume) >> 3;
		sound->cycle += cycleStep;
    }
}

void renderAudio(struct Memory* memory, int untilSamples)
{
	if (READ_REGISTER_DIRECT(memory, REG_NR52) & REG_NR52_ENABLED)
	{
		int stereoSelect = READ_REGISTER_DIRECT(memory, REG_NR51);
		int leftVolume = (READ_REGISTER_DIRECT(memory, REG_NR50) >> 4) & 0x7;
		int rightVolume = (READ_REGISTER_DIRECT(memory, REG_NR50) >> 0) & 0x7;
		
		if (GET_SQUARE_VOLUME(READ_REGISTER_DIRECT(memory, REG_NR12)) && (stereoSelect & 0x11))
		{
			renderSquareWave(
				&memory->audio, 
				untilSamples, 
				&memory->audio.sound1, 
				(stereoSelect & 0x01) ? rightVolume : 0,
				(stereoSelect & 0x10) ? leftVolume : 0
			);
		}
		
		if (GET_SQUARE_VOLUME(READ_REGISTER_DIRECT(memory, REG_NR22)) && (stereoSelect & 0x22))
		{
			renderSquareWave(
				&memory->audio, 
				untilSamples, 
				&memory->audio.sound2,
				(stereoSelect & 0x02) ? rightVolume : 0,
				(stereoSelect & 0x20) ? leftVolume : 0
			);
		}

		if (GET_PCM_VOLUME(READ_REGISTER_DIRECT(memory, REG_NR32)) && (stereoSelect & 0x33))
		{
			renderPatternWave(
				&memory->audio,
				untilSamples,
				&memory->audio.pcmSound,
				(stereoSelect & 0x03) ? rightVolume : 0,
				(stereoSelect & 0x30) ? leftVolume : 0
			);
		}
	}
	
	memory->audio.currentSampleIndex = untilSamples;
}

void updateAudio(struct Memory* memory, int apuTicks)
{
	struct AudioState* audio = &memory->audio;
	audio->sound1.waveDuty = GET_WAVE_DUTY(READ_REGISTER_DIRECT(memory, REG_NR11));
	audio->sound1.volume = GET_SQUARE_VOLUME(READ_REGISTER_DIRECT(memory, REG_NR12));
	audio->sound1.frequency = GET_SOUND_FREQ(READ_REGISTER_DIRECT(memory, REG_NR14), READ_REGISTER_DIRECT(memory, REG_NR13));

	audio->sound2.waveDuty = GET_WAVE_DUTY(READ_REGISTER_DIRECT(memory, REG_NR21));
	audio->sound2.volume = GET_SQUARE_VOLUME(READ_REGISTER_DIRECT(memory, REG_NR22));
	audio->sound2.frequency = GET_SOUND_FREQ(READ_REGISTER_DIRECT(memory, REG_NR24), READ_REGISTER_DIRECT(memory, REG_NR23));

	audio->pcmSound.volume = GET_PCM_VOLUME(READ_REGISTER_DIRECT(memory, REG_NR32));
	audio->pcmSound.frequency = GET_SOUND_FREQ(READ_REGISTER_DIRECT(memory, REG_NR34), READ_REGISTER_DIRECT(memory, REG_NR33));

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
		if (audio->currentWriteBuffer == audio->nextPlayBuffer)
		{
			renderAudio(memory, audio->samplesPerBuffer);
			audio->currentWriteBuffer = (audio->currentWriteBuffer + 1) % AUDIO_BUFFER_COUNT;
        	zeroMemory(audio->buffers[audio->currentWriteBuffer], audio->samplesPerBuffer * sizeof(struct AudioSample));
			audio->currentSampleIndex = 0;
		}

		osAiSetNextBuffer(audio->buffers[audio->nextPlayBuffer], audio->samplesPerBuffer * sizeof(struct AudioSample));
		audio->nextPlayBuffer = (audio->nextPlayBuffer + 1) % AUDIO_BUFFER_COUNT;
		++pendingBufferCount;
	}

}