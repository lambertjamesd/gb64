#include "audio.h"

void initAudio(struct AudioState* audioState, int frequency)
{
	audioState->frequency = osAiSetFrequency(frequency);
}
