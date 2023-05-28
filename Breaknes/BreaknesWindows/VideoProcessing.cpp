#include "pch.h"

VideoRender::VideoRender(SDL_Window* window, SDL_Surface* surface)
{
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
	int w = 256;
	int h = 240;

	Uint32* const pixels = (Uint32*)output_surface->pixels;

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			Uint32 color = field[w * y + x];

			// Upscale x2
			pixels[2 * x + (2 * y * output_surface->w)] = color;
			pixels[2 * x + 1 + (2 * y * output_surface->w)] = color;
			pixels[2 * x + ((2 * y + 1) * output_surface->w)] = color;
			pixels[2 * x + 1 + ((2 * y + 1) * output_surface->w)] = color;
		}
	}

	SDL_UpdateWindowSurface(output_window);

	field_counter++;
	printf("field: %lld\n", field_counter);
}
