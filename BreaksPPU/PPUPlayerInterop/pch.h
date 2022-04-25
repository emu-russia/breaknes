#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <cstdint>

#include "../../Breaknes/BaseLogicLib/BaseLogic.h"
#include "../PPUSim/ppu.h"
#include "../../Breaknes/BreaksCore/SRAM.h"
#include "../../Breaknes/BreaksCore/LS373.h"

#include "NROM.h"
#include "PPUPlayerBoard.h"
