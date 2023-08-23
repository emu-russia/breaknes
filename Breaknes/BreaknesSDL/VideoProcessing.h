
#pragma once

class VideoRender
{
	const int ScaleFactor = 2;
	const int SCREEN_WIDTH = 256;
	const int SCREEN_HEIGHT = 240;

	PPUSim::VideoSignalFeatures ppu_features{};
	int SamplesPerScan = 0;
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

	int field_counter = 0;

public:
	VideoRender();
	~VideoRender();

	void ProcessSample(PPUSim::VideoOutSignal& sample);
};
