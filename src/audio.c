#include "audio.h"
#include "memory_map.h"
#include "../memory.h"
#include <ultra64.h>

#include "debug_out.h"

#define ENABLE_AUDIO 0

u8 wavePattern[4][8] = {
    {0, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 1, 1},
};

short volumeTranslate[16] = {
	0x0000, 0x0222, 0x0444, 0x0666, 
	0x0888, 0x0AAA, 0x0CCC, 0x0EEE, 
	0x1111, 0x1333, 0x1555, 0x1777, 
	0x1999, 0x1BBB, 0x1DDD, 0x2000, 
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

	audioState->noiseSound.lfsr = 0x7FFF;
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
	
	int cycleStep = CYCLE_STEP(sound->frequency, state->sampleRate);
    struct AudioSample* output = state->buffers[state->currentWriteBuffer];

    for (sampleIndex = state->currentSampleIndex; sampleIndex < untilSamples; ++sampleIndex)
    {
		// fixed width mulitply the volume
		short volumeLevel = volumeTranslate[sound->envelope.volume];
		short sample = wavePattern[sound->waveDuty][sound->cycle >> 13] ? volumeLevel : -volumeLevel;
		output[sampleIndex].l += (sample * leftVolume) >> 3;
		output[sampleIndex].r += (sample * rightVolume) >> 3;
		sound->cycle += cycleStep;
    }
}

void renderPatternWave(
	struct Memory* memory,
	struct AudioState* state,
	int untilSamples,
	struct PCMSound* sound,
	int rightVolume,
	int leftVolume
) {
    int sampleIndex;
	int cycleStep = PCM_CYCLE_STEP(sound->frequency, state->sampleRate);
    struct AudioSample* output = state->buffers[state->currentWriteBuffer];

    for (sampleIndex = state->currentSampleIndex; sampleIndex < untilSamples; ++sampleIndex)
    {
		int pcmIndex = sound->cycle >> 12;
		unsigned char index =  GET_REGISTER_ADDRESS(memory, REG_WAVE_PAT)[pcmIndex];

		short sample = pcmTranslate[((sound->cycle >> 11) & 0x1) ? index & 0xF : ((index >> 4) & 0xF)] >> (sound->volume - 1);
		output[sampleIndex].l += (sample * leftVolume) >> 3;
		output[sampleIndex].r += (sample * rightVolume) >> 3;
		sound->cycle += cycleStep;
    }
}

/**
 * freq = 524288 / r / 2^s+1  calcs/second r=0.5 when dividingRatio = 0
 * sampleRate = samples/second
 * calcs/sample = freq / sampleRate
 */
u32 noiseSampleStep(int dividingRatio, int shift, int sampleRate)
{
	if (shift > NOISE_MAX_CLOCK_SHIFT)
	{
		return 0;
	}

	u32 result;

	if (dividingRatio)
	{
		result = 0x8000000 / (dividingRatio * sampleRate);
	}
	else
	{
		result = 0x8000000 * 2 / sampleRate;
	}

	result <<= (16 - (shift + 1));

	return result;
}

void renderNoise(
	struct AudioState* state,
	int untilSamples,
	struct NoiseSound* sound,
	int rightVolume,
	int leftVolume
) {
    int sampleIndex;
	u16 lfsr = sound->lfsr;
	enum LFSRWidth lfsrWidth = sound->lfsrWidth;
	struct AudioSample* output = state->buffers[state->currentWriteBuffer];
    for (sampleIndex = state->currentSampleIndex; sampleIndex < untilSamples; ++sampleIndex)
    {
		short volumeLevel = volumeTranslate[sound->envelope.volume];
		short sample = (lfsr & 1) ? -volumeLevel : volumeLevel;
		output[sampleIndex].l += (sample * leftVolume) >> 3;
		output[sampleIndex].r += (sample * rightVolume) >> 3;

		sound->accumulator += sound->sampleStep;

		while (sound->accumulator >= 0x1000000)
		{
			u16 bit = (lfsr ^ (lfsr >> 1)) & 1;
			if (lfsrWidth == LFSRWidth15) {
				lfsr = ((lfsr >> 1) & ~0x4000) | (bit << 14);
			} else {
				lfsr = ((lfsr >> 1) & ~0x40) | (bit << 6);
			}

			sound->accumulator -= 0x1000000;
		}
	}
	sound->lfsr = lfsr;
}

void renderAudio(struct Memory* memory, int untilSamples)
{
	if (READ_REGISTER_DIRECT(memory, REG_NR52) & REG_NR52_ENABLED)
	{
		int stereoSelect = READ_REGISTER_DIRECT(memory, REG_NR51);
		int leftVolume = (READ_REGISTER_DIRECT(memory, REG_NR50) >> 4) & 0x7;
		int rightVolume = (READ_REGISTER_DIRECT(memory, REG_NR50) >> 0) & 0x7;
		
		if (memory->audio.sound1.envelope.volume && (stereoSelect & 0x11))
		{
			renderSquareWave(
				&memory->audio, 
				untilSamples, 
				&memory->audio.sound1, 
				(stereoSelect & 0x01) ? rightVolume : 0,
				(stereoSelect & 0x10) ? leftVolume : 0
			);
		}
		
		if (memory->audio.sound2.envelope.volume && (stereoSelect & 0x22))
		{
			renderSquareWave(
				&memory->audio, 
				untilSamples, 
				&memory->audio.sound2,
				(stereoSelect & 0x02) ? rightVolume : 0,
				(stereoSelect & 0x20) ? leftVolume : 0
			);
		}

		if (memory->audio.pcmSound.volume && (stereoSelect & 0x33))
		{
			renderPatternWave(
				memory,
				&memory->audio,
				untilSamples,
				&memory->audio.pcmSound,
				(stereoSelect & 0x03) ? rightVolume : 0,
				(stereoSelect & 0x30) ? leftVolume : 0
			);
		}

		if (memory->audio.noiseSound.envelope.volume && 
			memory->audio.noiseSound.sampleStep && 
			(stereoSelect & 0x44)
		)
		{
			renderNoise(
				&memory->audio,
				untilSamples,
				&memory->audio.noiseSound,
				(stereoSelect & 0x04) ? rightVolume : 0,
				(stereoSelect & 0x40) ? leftVolume : 0
			);
		}
	}
	
	memory->audio.currentSampleIndex = untilSamples;
}

void advanceWriteBuffer(struct AudioState* audio)
{
	audio->currentWriteBuffer = (audio->currentWriteBuffer + 1) % AUDIO_BUFFER_COUNT;
	zeroMemory(audio->buffers[audio->currentWriteBuffer], audio->samplesPerBuffer * sizeof(struct AudioSample));
	audio->currentSampleIndex = 0;
}

void tickEnvelope(struct AudioEnvelope* envelope)
{
	if (envelope->stepDuration)
	{
		if (envelope->stepTimer == 0)
		{
			if (!(envelope->volume == 0 && envelope->step == 0xFF) && !(envelope->volume == 0xF && envelope->step == 0x01))
			{
				envelope->volume = (envelope->volume + envelope->step) & 0xF;
				envelope->stepTimer = envelope->stepDuration - 1;
			}
		}
		else
		{
			--envelope->stepTimer;
		}
	}
}

u16 tickSweep(struct AudioSweep* envelope, u16 frequency)
{
	if (envelope->stepDuration)
	{
		if (envelope->stepTimer == 0)
		{
			// TODO clamping
			u16 stepAmount = frequency >> envelope->stepShift;
			if (envelope->stepDir)
			{
				frequency -= stepAmount;
			}
			else
			{
				frequency += stepAmount;
			}
			envelope->stepTimer = envelope->stepDuration - 1;
		}
		else
		{
			--envelope->stepTimer;
		}
	}

	return frequency;
}

void tickSquareWave(struct SquareWaveSound* squareWave)
{
	if (squareWave->length)
	{
		TICK_LENGTH(squareWave->length);
		squareWave->frequency = tickSweep(&squareWave->sweep, squareWave->frequency);
		tickEnvelope(&squareWave->envelope);
	}
}

void tickPCM(struct PCMSound* pcmSound)
{
	if (pcmSound->length)
	{
		TICK_LENGTH(pcmSound->length);
	}
}

void tickNoise(struct NoiseSound* noiseSound)
{
	if (noiseSound->length)
	{
		TICK_LENGTH(noiseSound->length);
		tickEnvelope(&noiseSound->envelope);
	}
}

void tickAudio(struct Memory* memory, int untilCyles)
{
	struct AudioState* audio = &memory->audio;

	while (audio->cyclesEmulated < untilCyles)
	{
		int tickTo = audio->currentSampleIndex + (audio->sampleRate >> APU_TICKS_PER_SEC_L2);

		if (tickTo > audio->samplesPerBuffer)
		{
			renderAudio(memory, audio->samplesPerBuffer);
			advanceWriteBuffer(audio);
			tickTo -= audio->samplesPerBuffer;
		}
		renderAudio(memory, tickTo);
		tickSquareWave(&audio->sound1);
		tickSquareWave(&audio->sound2);
		tickPCM(&audio->pcmSound);
		tickNoise(&audio->noiseSound);

		audio->cyclesEmulated += CYCLES_PER_TICK;
	}

	updateOnOffRegister(memory);
}

void initEnvelope(struct AudioEnvelope* target, unsigned char envelopeData)
{
	target->volume = GET_ENVELOPE_VOLUME(envelopeData);
	target->stepDuration = GET_ENVELOPE_STEP_DURATION(envelopeData) << 2;
	target->stepTimer = target->stepDuration - 1;
	target->step = GET_ENVELOPE_DIR(envelopeData) ? 1 : -1;
}

void initSweep(struct AudioSweep* sweep, unsigned char sweepData)
{
	sweep->stepDir = GET_SWEEP_DIR(sweepData);
	sweep->stepShift = GET_SWEEP_SHIFT(sweepData);
	sweep->stepDuration = GET_SWEEP_TIME(sweepData) << 1;
	sweep->stepTimer = sweep->stepDuration - 1;
}

void restartSound(struct Memory* memory, int currentCycle, enum SoundIndex soundNumber)
{
#if ENABLE_AUDIO
	tickAudio(memory, currentCycle);
	struct AudioState* audio = &memory->audio;

	switch (soundNumber)
	{
		case SoundIndexSquare1:
			initSweep(&audio->sound1.sweep, READ_REGISTER_DIRECT(memory, REG_NR10));
			audio->sound1.waveDuty = GET_WAVE_DUTY(READ_REGISTER_DIRECT(memory, REG_NR11));
			audio->sound1.length = GET_SOUND_LENGTH_LIMITED(READ_REGISTER_DIRECT(memory, REG_NR14)) ? 
				GET_SOUND_LENGTH(READ_REGISTER_DIRECT(memory, REG_NR11)) :
				SOUND_LENGTH_INDEFINITE;
			initEnvelope(&audio->sound1.envelope, READ_REGISTER_DIRECT(memory, REG_NR12));
			audio->sound1.frequency = GET_SOUND_FREQ(READ_REGISTER_DIRECT(memory, REG_NR14), READ_REGISTER_DIRECT(memory, REG_NR13));
			break;
		case SoundIndexSquare2:
			initSweep(&audio->sound2.sweep, 0);
			audio->sound2.waveDuty = GET_WAVE_DUTY(READ_REGISTER_DIRECT(memory, REG_NR21));
			audio->sound2.length = GET_SOUND_LENGTH_LIMITED(READ_REGISTER_DIRECT(memory, REG_NR24)) ? 
				GET_SOUND_LENGTH(READ_REGISTER_DIRECT(memory, REG_NR21)) :
				SOUND_LENGTH_INDEFINITE;
			initEnvelope(&audio->sound2.envelope, READ_REGISTER_DIRECT(memory, REG_NR22));
			audio->sound2.frequency = GET_SOUND_FREQ(READ_REGISTER_DIRECT(memory, REG_NR24), READ_REGISTER_DIRECT(memory, REG_NR23));
			break;
		case SoundIndexPCM:
			audio->pcmSound.cycle = 0;
			audio->pcmSound.volume = GET_PCM_VOLUME(READ_REGISTER_DIRECT(memory, REG_NR32));
			audio->pcmSound.frequency = GET_SOUND_FREQ(READ_REGISTER_DIRECT(memory, REG_NR34), READ_REGISTER_DIRECT(memory, REG_NR33));
			audio->pcmSound.length = GET_SOUND_LENGTH_LIMITED(READ_REGISTER_DIRECT(memory, REG_NR34)) ?
				READ_REGISTER_DIRECT(memory, REG_NR31) :
				SOUND_LENGTH_INDEFINITE;
			break;
		case SoundIndexNoise:
			audio->noiseSound.lfsr = 0x7FFF;
			audio->noiseSound.accumulator = 0;
			audio->noiseSound.sampleStep = noiseSampleStep(
				READ_REGISTER_DIRECT(memory, READ_REGISTER_DIRECT(memory, REG_NR43) & 0x3), 
				READ_REGISTER_DIRECT(memory, REG_NR43) >> 4,
				audio->sampleRate
			);
			initEnvelope(&audio->noiseSound.envelope, READ_REGISTER_DIRECT(memory, REG_NR42));
			audio->noiseSound.length = GET_SOUND_LENGTH_LIMITED(READ_REGISTER_DIRECT(memory, REG_NR44)) ? 
				GET_SOUND_LENGTH(READ_REGISTER_DIRECT(memory, REG_NR41)) :
				SOUND_LENGTH_INDEFINITE;
			break;
	}
	updateOnOffRegister(memory);
#endif
}

void finishAudioFrame(struct Memory* memory)
{
#if ENABLE_AUDIO
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
		if (audio->currentWriteBuffer == audio->nextPlayBuffer)
		{
			renderAudio(memory, audio->samplesPerBuffer);
			advanceWriteBuffer(audio);
		}

		osAiSetNextBuffer(audio->buffers[audio->nextPlayBuffer], audio->samplesPerBuffer * sizeof(struct AudioSample));
		audio->nextPlayBuffer = (audio->nextPlayBuffer + 1) % AUDIO_BUFFER_COUNT;
		++pendingBufferCount;
	}
#endif
}

void updateOnOffRegister(struct Memory* memory)
{
	u8 existingValue = READ_REGISTER_DIRECT(memory, REG_NR52);
	u8 result = existingValue & REG_NR52_ENABLED;
	struct AudioState* audio = &memory->audio;

	if (existingValue & REG_NR52_ENABLED)
	{
		if (audio->sound1.length)
		{
			result |= 0x01;
		}
		if (audio->sound2.length)
		{
			result |= 0x02;
		}
		if (audio->pcmSound.length)
		{
			result |= 0x03;
		}
		if (audio->noiseSound.length)
		{
			result |= 0x04;
		}
	}

	WRITE_REGISTER_DIRECT(memory, REG_NR52, result);
}