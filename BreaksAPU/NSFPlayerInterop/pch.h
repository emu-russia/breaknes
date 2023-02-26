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
#include "../../Common/DebugHub/DebugHub.h"
#include "../../Common/BaseBoardLib/SRAM.h"
#include "../../Common/BaseBoardLib/Fake6502.h"

#include "NSFMapper.h"
#include "AbstractBoard.h"
#include "NSFPlayerBoard.h"
#include "APUPlayerBoard.h"
#include "BoardFactory.h"
