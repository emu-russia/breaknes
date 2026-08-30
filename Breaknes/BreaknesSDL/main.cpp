#include "pch.h"

bool run_worker = false;

VideoRender* vid_out;
SoundOutput* snd_out;

/// <summary>
/// The main thread, which simply performs Step and updates the video/audio as needed.
/// </summary>
/// <returns></returns>
int SDLCALL MainWorker (void* data)
{
	while (run_worker) {

		Step();

		if (!InResetState())
		{
#if !CONSOLE_ONLY
			PPUSim::VideoOutSignal sample;
			SampleVideoSignal(&sample);
			vid_out->ProcessSample(sample);
			snd_out->FeedSample();
#endif
		}
	}

	return 0;
}

static void PrintUsage()
{
	printf("Use: breaknes [--ioconfig [config_path]] <file.nes>\n");
	printf("     --ioconfig [config_path] - launch the IO configurator (issue #516),\n");
	printf("                                 default config file: IOConfigSDL.json\n");
}

int main(int argc, char ** argv) {

	SDL_Thread* worker{};

	// Command line parsing (issue #516):
	//   breaknes --ioconfig [config_path]  - launch the IO configurator and exit
	//   breaknes <file.nes>                - run the emulation

	bool config_mode = false;
	const char* config_path = BreaknesSDL::kDefaultIOConfigPath;
	const char* rom_path = nullptr;

	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "--ioconfig")
		{
			config_mode = true;

			// An optional config file path may follow as the next argument
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				config_path = argv[++i];
			}
		}
		else
		{
			rom_path = argv[i];
		}
	}

	if (!config_mode && rom_path == nullptr)
	{
		PrintUsage();
		return -1;
	}

	CreateBoard ((char*)BreaknesSDL::kSDLBoardName, (char*)BreaknesSDL::kSDLBoardAPU, (char*)BreaknesSDL::kSDLBoardPPU, (char*)BreaknesSDL::kSDLBoardP1);
	Reset();

	// The IO configurator requires a created board (for the actuator enumeration
	// via BreaksCore) and an SDL window (to receive keyboard events).

	if (config_mode)
	{
#if !CONSOLE_ONLY
		vid_out = new VideoRender();
#endif
		printf("Launching IO configurator...\n");
		BreaknesSDL::RunIOConfigurator(config_path);
#if !CONSOLE_ONLY
		delete vid_out;
		vid_out = nullptr;
#endif
		DestroyBoard();
		return 0;
	}

	printf("Loading ROM: %s\n", rom_path);

	// CartPcb identification: the Nescartdb data is copied next to the executable
	// at build time; run the emulator from its own directory (or set NESCARDB_DIR).
	const char* nescartdb_dir = getenv("NESCARDB_DIR");
	SetNescartdbDir((char*)(nescartdb_dir ? nescartdb_dir : "Nescartdb"));

	// Make additional settings for emulation in the Breaknes casual environment

	SetOamDecayBehavior(PPUSim::OAMDecayBehavior::Keep);
	SetRAWColorMode(true);

	FILE* f = fopen(rom_path, "rb");
	if (!f) {
		printf("Cannot load: %s\n", rom_path);
		return -2;
	}

	fseek(f, 0, SEEK_END);
	auto nes_image_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t* nes_image = new uint8_t[nes_image_size];

	auto readed = fread(nes_image, 1, nes_image_size, f);
	if (readed != nes_image_size) {
		printf("Wrong nes file size!\n");
		return -3;
	}
	fclose(f);

	if (InsertCartridge(nes_image, nes_image_size) < 0) {
		printf("InsertCartridge failed! The board was not identified in Nescartdb (check that Nescartdb/ is next to the executable).\n");
		return -4;
	}

	bool quit = false;

#if !CONSOLE_ONLY
	vid_out = new VideoRender();
	snd_out = new SoundOutput();
#endif

	// IO subsystem: create the device instances from the IO settings and attach
	// them to the motherboard ports (issue #516).

	BreaknesSDL::IOProcessor io;
	io.AttachDevicesToBoard(BreaknesSDL::kSDLBoardName, config_path);

	// Run the main thread, which will emulate the system

	run_worker = true;
	worker = SDL_CreateThread(MainWorker, "work thread", 0);

	// Start the SDL event loop together with the main thread.
	// The SDL input events (keyboard, game controllers) are forwarded to the
	// IO processor, which dispatches them to the attached devices.

	while (!quit) {
			
		SDL_Delay(1);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			io.ProcessEvent(event);
			if (event.type == SDL_QUIT) {
				quit = true;
			}
		}

		io.Dispatch();
	}

	run_worker = false;
	SDL_WaitThread(worker, 0);

	io.DetachDevicesFromBoard();

#if !CONSOLE_ONLY
	delete vid_out;
	delete snd_out;
#endif

	EjectCartridge();
	delete[] nes_image;
	DestroyBoard();
    
	return 0;
}
