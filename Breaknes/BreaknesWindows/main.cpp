#include "pch.h"

const int SCREEN_WIDTH = 256*2;
const int SCREEN_HEIGHT = 240*2;

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
			PPUSim::VideoOutSignal sample;
			SampleVideoSignal(&sample);
			vid_out->ProcessSample(sample);
			snd_out->FeedSample();
		}

	}

	return 0;
}

int main(int argc, char ** argv) {

	SDL_Thread* worker{};

	if (argc <= 1) {
		printf("Use: breaknes <file.nes>\n");
		return -1;
	}
	else {
		printf("Loading ROM: %s\n", argv[1]);
	}

	CreateBoard ((char *)"NESBoard", (char*)"RP2A03G", (char*)"RP2C02G", (char*)"NES");
	Reset();

	// Make additional settings for emulation in the Breaknes casual environment

	SetOamDecayBehavior(PPUSim::OAMDecayBehavior::Keep);
	SetRAWColorMode(true);

	FILE* f = fopen(argv[1], "rb");
	if (!f) {
		printf("Cannot load: %s\n", argv[1]);
		return -2;
	}

	fseek(f, 0, SEEK_END);
	auto nes_image_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t* nes_image = new uint8_t[nes_image_size];

	fread(nes_image, 1, nes_image_size, f);
	fclose(f);

	if (InsertCartridge(nes_image, nes_image_size) < 0) {
		printf("InsertCartridge failed!\n");
		return -3;
	}

	if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0) {
		std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
	} else {

		SDL_Window* window = SDL_CreateWindow(
			"Breaknes",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			0);

		if (window == NULL) {
			printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
			return -4;
		}

		SDL_Surface* surface = SDL_GetWindowSurface(window);

		if (surface == NULL) {
			printf("SDL_GetWindowSurface failed: %s\n", SDL_GetError());
			return -5;
		}

		// Initialize window to all black
		//SDL_FillSurfaceRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
		SDL_UpdateWindowSurface(window);
		
		bool quit = false;

		vid_out = new VideoRender(window, surface);
		snd_out = new SoundOutput();

		// Run the main thread, which will emulate the system

		run_worker = true;
		worker = SDL_CreateThread(MainWorker, "work thread", 0);

		// Start the SDL event loop together with the main thread

		while (!quit) {
			
			SDL_Delay(1);

			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					quit = true;
				}
			}
		}

		run_worker = false;
		SDL_WaitThread(worker, 0);

		delete vid_out;
		delete snd_out;

		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	EjectCartridge();
	delete[] nes_image;
	DestroyBoard();
   
	return 0;
}
