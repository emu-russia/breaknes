#include "pch.h"

VideoRender::VideoRender()
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		printf("SDL video could not initialize! SDL_Error: %s\n", SDL_GetError());
	}

	SDL_Window* window = SDL_CreateWindow(
		"Breaknes",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH * ScaleFactor, SCREEN_HEIGHT * ScaleFactor,
		0);

	if (window == NULL) {
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		return;
	}

	SDL_Surface* surface = SDL_GetWindowSurface(window);

	if (surface == NULL) {
		printf("SDL_GetWindowSurface failed: %s\n", SDL_GetError());
		return;
	}

	// Initialize window to all black
	//SDL_FillSurfaceRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
	SDL_UpdateWindowSurface(window);

	output_window = window;
	output_surface = surface;

	GetPpuSignalFeatures(&ppu_features);

	SamplesPerScan = ppu_features.PixelsPerScan * ppu_features.SamplesPerPCLK;
	ScanBuffer = new PPUSim::VideoOutSignal[2 * SamplesPerScan];
	memset(ScanBuffer, 0, 2 * SamplesPerScan);
	WritePtr = 0;

	SyncFound = false;
	SyncPos = -1;
	CurrentScan = 0;

	field = new uint32_t[256 * 240];
	memset(field, 0, 256 * 240);
}

VideoRender::~VideoRender()
{
	SDL_DestroyWindow(output_window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	delete[] ScanBuffer;
	delete[] field;
}

void VideoRender::ProcessSample(PPUSim::VideoOutSignal& sample)
{
	ScanBuffer[WritePtr] = sample;

	// Check that the sample is HSync.

	bool sync = false;
	sync = sample.RAW.Sync != 0;

	// If the beginning of HSync is found - remember its offset in the input buffer.

	if (sync && !SyncFound)
	{
		SyncPos = WritePtr;
		SyncFound = true;
	}

	// Advance write pointer

	WritePtr++;

	// If HSync is found and the number of samples is more than enough, process the scan.

	if (SyncFound && (SyncPos + WritePtr) >= SamplesPerScan)
	{
		ProcessScanRAW();

		SyncFound = false;
		WritePtr = 0;
	}

	if (WritePtr >= 2 * SamplesPerScan)
	{
		SyncFound = false;
		WritePtr = 0;
	}
}

void VideoRender::ProcessScanRAW()
{
	int ReadPtr = SyncPos;

	// Skip HSync and Back Porch

	while (ScanBuffer[ReadPtr].RAW.Sync != 0)
	{
		ReadPtr++;
	}

	ReadPtr += ppu_features.BackPorchSize * ppu_features.SamplesPerPCLK;

	// Output the visible part of the signal

	for (int i = 0; i < 256; i++)
	{
		if (CurrentScan < 240)
		{
			uint8_t r, g, b;
			ConvertRAWToRGB(ScanBuffer[ReadPtr].RAW.raw, &r, &g, &b);

			field[CurrentScan * 256 + i] = SDL_MapRGB(output_surface->format, r, g, b);
		}

		ReadPtr += ppu_features.SamplesPerPCLK;
	}

	CurrentScan++;
	if (CurrentScan >= ppu_features.ScansPerField)
	{
		VisualizeField();
		CurrentScan = 0;
	}
}

void VideoRender::VisualizeField()
{
	int w = SCREEN_WIDTH;
	int h = SCREEN_HEIGHT;

	Uint32* const pixels = (Uint32*)output_surface->pixels;

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			Uint32 color = field[w * y + x];

			for (int s = 0; s < ScaleFactor; s++) {
				for (int t = 0; t < ScaleFactor; t++) {
					pixels[ScaleFactor * x + s + ((ScaleFactor * y + t) * output_surface->w)] = color;
				}
			}
		}
	}

	SDL_UpdateWindowSurface(output_window);

	field_counter++;
	printf("field: %d\n", field_counter);
}
