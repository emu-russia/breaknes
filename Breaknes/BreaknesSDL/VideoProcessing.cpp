#include "pch.h"

VideoRender::VideoRender(int _tv_count, const char* layout)
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		printf("SDL video could not initialize! SDL_Error: %s\n", SDL_GetError());
		return;
	}

	tv_count = _tv_count < 1 ? 1 : (_tv_count > 2 ? 2 : _tv_count);
	tv_layout = (layout != nullptr && strcmp(layout, "vertical") == 0) ? TvLayout::Vertical : TvLayout::Horizontal;

	// The window size depends on the number of TVs and their physical layout.
	int window_w = SCREEN_WIDTH * ScaleFactor * (tv_layout == TvLayout::Horizontal ? tv_count : 1);
	int window_h = SCREEN_HEIGHT * ScaleFactor * (tv_layout == TvLayout::Vertical ? tv_count : 1);

	SDL_Window* window = SDL_CreateWindow(
		"Breaknes",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		window_w, window_h,
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
	SDL_UpdateWindowSurface(window);

	output_window = window;
	output_surface = surface;

	// Initialize one renderer state per TV. The signal features (and the palette)
	// come from the PPU the TV is bound to.
	for (int tv = 0; tv < tv_count; tv++)
	{
		TvState& st = tvs[tv];

		st.ppu_index = GetTVBinding(tv);
		if (st.ppu_index < 0)
			st.ppu_index = 0;

		GetPpuSignalFeaturesEx(st.ppu_index, &st.ppu_features);

		st.SamplesPerScan = st.ppu_features.PixelsPerScan * st.ppu_features.SamplesPerPCLK;
		st.ScanBuffer = new PPUSim::VideoOutSignal[2 * st.SamplesPerScan];
		memset(st.ScanBuffer, 0, 2 * st.SamplesPerScan * sizeof(PPUSim::VideoOutSignal));
		st.WritePtr = 0;

		st.SyncFound = false;
		st.SyncPos = -1;
		st.CurrentScan = 0;

		st.field = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];
		memset(st.field, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
	}
}

VideoRender::~VideoRender()
{
	SDL_DestroyWindow(output_window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	for (int tv = 0; tv < tv_count; tv++)
	{
		delete[] tvs[tv].ScanBuffer;
		delete[] tvs[tv].field;
	}
}

void VideoRender::ProcessSample(int tv, PPUSim::VideoOutSignal& sample)
{
	if (tv < 0 || tv >= tv_count)
		return;

	TvState& st = tvs[tv];
	st.ScanBuffer[st.WritePtr] = sample;

	// Check that the sample is HSync.

	bool sync = false;
	sync = sample.RAW.Sync != 0;

	// If the beginning of HSync is found - remember its offset in the input buffer.

	if (sync && !st.SyncFound)
	{
		st.SyncPos = st.WritePtr;
		st.SyncFound = true;
	}

	// Advance write pointer

	st.WritePtr++;

	// If HSync is found and the number of samples is more than enough, process the scan.

	if (st.SyncFound && (st.SyncPos + st.WritePtr) >= st.SamplesPerScan)
	{
		ProcessScanRAW(tv);

		st.SyncFound = false;
		st.WritePtr = 0;
	}

	if (st.WritePtr >= 2 * st.SamplesPerScan)
	{
		st.SyncFound = false;
		st.WritePtr = 0;
	}
}

void VideoRender::ProcessScanRAW(int tv)
{
	TvState& st = tvs[tv];
	int ReadPtr = st.SyncPos;

	// Skip HSync and Back Porch

	while (st.ScanBuffer[ReadPtr].RAW.Sync != 0)
	{
		ReadPtr++;
	}

	ReadPtr += st.ppu_features.BackPorchSize * st.ppu_features.SamplesPerPCLK;

	// Output the visible part of the signal

	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		if (st.CurrentScan < SCREEN_HEIGHT)
		{
			uint8_t r, g, b;
			ConvertRAWToRGBEx(st.ppu_index, st.ScanBuffer[ReadPtr].RAW.raw, &r, &g, &b);

			st.field[st.CurrentScan * SCREEN_WIDTH + i] = SDL_MapRGB(output_surface->format, r, g, b);
		}

		ReadPtr += st.ppu_features.SamplesPerPCLK;
	}

	st.CurrentScan++;
	if (st.CurrentScan >= st.ppu_features.ScansPerField)
	{
		VisualizeField();
		st.CurrentScan = 0;
	}
}

void VideoRender::VisualizeField()
{
	int w = SCREEN_WIDTH;
	int h = SCREEN_HEIGHT;

	Uint32* const pixels = (Uint32*)output_surface->pixels;

	// Clear the whole surface first (letterboxing between the TVs / aspect margins).
	SDL_FillRect(output_surface, NULL, SDL_MapRGB(output_surface->format, 0, 0, 0));

	// Draw the field of every TV at its place in the window according to the layout.
	// When all TVs show the same PPU (e.g. the debug "one PPU to two TVs" binding)
	// their fields are completed synchronously.
	for (int tv = 0; tv < tv_count; tv++)
	{
		TvState& st = tvs[tv];
		int base_x = (tv_layout == TvLayout::Horizontal) ? tv * w * ScaleFactor : 0;
		int base_y = (tv_layout == TvLayout::Vertical) ? tv * h * ScaleFactor : 0;

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				Uint32 color = st.field[w * y + x];

				for (int s = 0; s < ScaleFactor; s++) {
					for (int t = 0; t < ScaleFactor; t++) {
						pixels[base_x + ScaleFactor * x + s + ((base_y + ScaleFactor * y + t) * output_surface->w)] = color;
					}
				}
			}
		}
	}

	SDL_UpdateWindowSurface(output_window);

	field_counter++;
	printf("field: %d\n", field_counter);
}
