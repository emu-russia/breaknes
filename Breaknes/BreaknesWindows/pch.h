#pragma once

#define CONSOLE_ONLY 1			// Used for debugging under WSL2

//#define SDL_MAIN_HANDLED
#ifdef _WIN32
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif
#include <iostream>

#include "../BreaksCore/BreaksCore.h"
#include "VideoProcessing.h"
#include "SoundProcessing.h"
