#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <list>
#include <cassert>

#include "../../Common/BaseLogicLib/BaseLogic.h"
#include "../../Breaks6502/M6502Core/core.h"
#include "../APUSim/apu.h"
#include "../../Breaknes/BreaksCore/CoreApi.h"

#include "APUDebug.h"
#include "BankedSRAM.h"
#include "NSFPlayerBoard.h"
