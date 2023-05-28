
#pragma once

class VideoRender
{
	PPUSim::VideoSignalFeatures ppu_features{};
	int SamplesPerScan;
	bool RawMode = false;

	PPUSim::VideoOutSignal* ScanBuffer = nullptr;
	int WritePtr = 0;
	bool SyncFound = false;
	int SyncPos = -1;

	uint32_t* field = nullptr;
	int CurrentScan = 0;

	SDL_Surface* output_surface = nullptr;
	SDL_Window* output_window = nullptr;

	void ProcessScanRAW();
	void VisualizeField();

	uint64_t field_counter = 0;

public:
	VideoRender(SDL_Window* window, SDL_Surface* surface);
	~VideoRender();

	void ProcessSample(PPUSim::VideoOutSignal& sample);
};
