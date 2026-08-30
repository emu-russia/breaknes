
#pragma once

/// <summary>
/// Physical arrangement of the TV Sets in the emulator window.
/// </summary>
enum class TvLayout
{
	Horizontal,		// side by side
	Vertical,		// one above the other
};

/// <summary>
/// Renders the video signal of up to 2 TV Sets in one window (issue #515).
/// Each TV displays the signal of the PPU it is bound to (see BindPPUToTV).
/// </summary>
class VideoRender
{
	const int ScaleFactor = 2;
	const int SCREEN_WIDTH = 256;
	const int SCREEN_HEIGHT = 240;

	int tv_count = 1;
	TvLayout tv_layout = TvLayout::Horizontal;

	struct TvState
	{
		PPUSim::VideoSignalFeatures ppu_features{};
		int SamplesPerScan = 0;
		int ppu_index = -1;

		PPUSim::VideoOutSignal* ScanBuffer = nullptr;
		int WritePtr = 0;
		bool SyncFound = false;
		int SyncPos = -1;

		uint32_t* field = nullptr;
		int CurrentScan = 0;
		bool FieldReady = false;		// the field of this TV is complete in the current frame
	};

	TvState tvs[2];

	/// <summary>
	/// Number of TVs that have completed their field in the current frame.
	/// The window is rendered once per frame, when all TVs are ready.
	/// </summary>
	int fields_ready_count = 0;

	SDL_Surface* output_surface = nullptr;
	SDL_Window* output_window = nullptr;

	void ProcessScanRAW(int tv);
	void VisualizeField();

	int field_counter = 0;

public:
	/// <summary>
	/// Create a window with the given number of TV Sets (1 or 2) in the given layout.
	/// </summary>
	VideoRender(int tv_count = 1, const char* layout = "horizontal");
	~VideoRender();

	/// <summary>
	/// Feed one video sample of the given TV (sampled with SampleVideoSignalEx).
	/// </summary>
	void ProcessSample(int tv, PPUSim::VideoOutSignal& sample);
};
