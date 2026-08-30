#include "pch.h"

#include "BoardConfig.h"

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
			// Sample and render the video signal of every connected TV Set (issue #515).
			int tv_count = GetTVCount();
			for (int tv = 0; tv < tv_count; tv++)
			{
				PPUSim::VideoOutSignal sample;
				SampleVideoSignalEx(tv, &sample);
				vid_out->ProcessSample(tv, sample);
			}
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

	// Board configuration (issue #515): read the BoardDescription.json next to the
	// executable and look up our board by name (kSDLBoardName). This provides the
	// board name, the APU/PPU list, the TV layout and the TV<->PPU binding.

	BreaknesSDL::BoardConfig board_config;
	if (!BreaknesSDL::LoadBoardConfig("BoardDescription.json", board_config))
	{
		printf("Cannot load the board configuration: BoardDescription.json with the \"%s\" board entry is required next to the executable.\n", BreaknesSDL::kSDLBoardName);
		return -5;
	}

	if (board_config.ppus.empty())
	{
		printf("The \"%s\" board entry in BoardDescription.json does not specify any PPU (\"ppus\" array or \"ppu\" string).\n", BreaknesSDL::kSDLBoardName);
		return -5;
	}

	std::vector<char*> ppu_names;
	for (auto& ppu : board_config.ppus)
	{
		ppu_names.push_back((char*)ppu.c_str());
	}

	CreateBoardEx((char*)board_config.name.c_str(), (char*)board_config.apu.c_str(), ppu_names.data(), (int)ppu_names.size(), (char*)board_config.p1.c_str());

	// Apply the TV<->PPU binding from the JSON ("tvs": tvs[i] = PPU of the i-th TV).
	// The default binding (TV[i] shows PPU[i]) is set up by the core.
	for (size_t tv = 0; tv < board_config.tvs.size() && tv < 2; tv++)
	{
		BindPPUToTV(board_config.tvs[tv], (int)tv, true);
	}

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
	// One TV per connected TV Set, in the physical layout from the board description.
	vid_out = new VideoRender(GetTVCount(), board_config.tv_layout.c_str());
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
