#include "pch.h"

// Since the simulation is now very slow, the following strategy is used to reproduce the sound:
// Collect one second of sound. As it is recorded, play it back via sound driver and clear the receive buffer.

SoundOutput::SoundOutput()
{
	Redecimate();

	SampleBuf_Size = OutputSampleRate;
	SampleBuf = new int16_t[SampleBuf_Size];
	memset(SampleBuf, 0, SampleBuf_Size * sizeof(int16_t));
	SampleBuf_Ptr = 0;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		printf ("SDL audio could not initialize! SDL_Error: %s\n", SDL_GetError());
		return;
	}

	spec.freq = OutputSampleRate;
	spec.format = AUDIO_S16;
	spec.channels = 1;
	spec.samples = SampleBuf_Size;
	spec.callback = Mixer;
	spec.userdata = this;

	dev_id = SDL_OpenAudioDevice(NULL, 0, &spec, &spec_obtainted, 0);
	SDL_PauseAudioDevice(dev_id, 1);
}

SoundOutput::~SoundOutput()
{
	SDL_CloseAudioDevice(dev_id);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	delete[] SampleBuf;
}

void SoundOutput::Mixer(void* thisptr, Uint8* stream, int len)
{
	SoundOutput* snd_out = (SoundOutput*)thisptr;
	if (snd_out->Dma) {
		SDL_MixAudioFormat(stream, (const Uint8*)snd_out->SampleBuf, AUDIO_S16SYS, len, SDL_MIX_MAXVOLUME);
		snd_out->Dma = false;
	}
	else {
		memset(snd_out->SampleBuf, 0, snd_out->SampleBuf_Size * sizeof(int16_t));
		SDL_PauseAudioDevice(snd_out->dev_id, 1);
	}
}

void SoundOutput::FeedSample()
{
	if (DecimateCounter >= DecimateEach)
	{
		float sample;
		{
			SampleAudioSignal(&sample);
		}

		SampleBuf[SampleBuf_Ptr++] = (int16_t)(sample * (float)INT16_MAX);
		DecimateCounter = 0;

		if (SampleBuf_Ptr >= SampleBuf_Size)
		{
			Playback();
		}
	}
	else
	{
		DecimateCounter++;
	}
}

/// <summary>
/// If you change the sampling frequency of the sound source or output frequency, you must recalculate the decimation factor.
/// </summary>
void SoundOutput::Redecimate()
{
	APUSim::AudioSignalFeatures aux_features{};
	GetApuSignalFeatures(&aux_features);
	DecimateEach = aux_features.SampleRate / OutputSampleRate;
	printf ("APUSim sample rate: %d, SoundCard sample rate: %d, decimate factor: %d\n", aux_features.SampleRate, OutputSampleRate, DecimateEach);
	DecimateCounter = 0;
}

void SoundOutput::Playback()
{
	printf("Play 1 second\n");
	Dma = true;
	SDL_PauseAudioDevice(dev_id, 0);
	SampleBuf_Ptr = 0;
}
