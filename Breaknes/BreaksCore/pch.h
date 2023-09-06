#pragma once

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <cassert>
#include <memory.h>
#include <cstddef>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "../../Tools/Breakasm/ASM.h"
#include "../../Tools/Breakasm/ASMOPS.h"

// Very basic

#include "../../Common/BaseLogicLib/BaseLogic.h"
#include "../../Common/JsonLib/Json.h"

// Big chips

#include "../../Chips/M6502Core/core.h"
#include "../../Chips/APUSim/apu.h"
#include "../../Chips/PPUSim/ppu.h"

// Board logic

#include "../../Common/BaseBoardLib/LS139.h"
#include "../../Common/BaseBoardLib/LS368.h"
#include "../../Common/BaseBoardLib/LS373.h"
#include "../../Common/BaseBoardLib/SRAM.h"
#include "../../Common/BaseBoardLib/Fake6502.h"
#include "../../Common/BaseBoardLib/LS32.h"
#include "../../Common/BaseBoardLib/LS161.h"

// DebugHub to all

#include "DebugHub.h"

// Cartridge & Mappers

#include "../../Mappers/Mappers.h"

// IO Subsystem

#include "../../IO/IO.h"

// Boards

#include "RegDumpEmitter.h"
#include "SignalDefs.h"
#include "AbstractBoard.h"
#include "BogusBoard.h"
#include "NESBoard.h"
#include "FamicomBoard.h"
#include "BoardFactory.h"
#include "APUPlayerBoard.h"
#include "PPUPlayerBoard.h"

// To C#

#include "CoreApi.h"
