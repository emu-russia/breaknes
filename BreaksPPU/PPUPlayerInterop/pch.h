#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <list>
#include <cassert>

#include "../../Common/BaseLogicLib/BaseLogic.h"
#include "../PPUSim/ppu.h"
#include "../../Common/BaseBoardLib/SRAM.h"
#include "../../Common/BaseBoardLib/LS373.h"

#include "../../Breaks6502/M6502Core/core.h"
#include "../../Common/BaseBoardLib/Fake6502.h"

#include "../../Mappers/NES_Header.h"
#include "../../Mappers/NROM.h"

#include "../../Common/DebugHub/DebugHub.h"

#include "PPUPlayerBoard.h"

extern PPUPlayer::Board* board;
