#pragma once

class SoundOutput
{
	SDL_AudioDeviceID dev_id;

	static void SDLCALL Mixer (void* unused, Uint8* stream, int len);

	void Redecimate();
	void Playback();

	long DecimateEach;
	long DecimateCounter;
	const long OutputSampleRate = 48000;

	int16_t* SampleBuf;
	int SampleBuf_Ptr;
	int SampleBuf_Size;

	bool Dma = false;

public:
	SoundOutput();
	~SoundOutput();

	/// <summary>
	/// This sampler is used for SRC.
	/// The AUX output is sampled at a high frequency, which cannot be played by a ordinary sound card.
	/// Therefore, some of the samples are skipped to match the playback frequency.
	/// </summary>
	void FeedSample();
};
