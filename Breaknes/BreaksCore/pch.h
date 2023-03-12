#pragma once

#include <iostream>
#include <string>
#include <list>
#include <cassert>
#include <Windows.h>

#include "../../Breaks6502/Breakasm/ASM.h"
#include "../../Breaks6502/Breakasm/ASMOPS.h"

// Very basic

#include "../../Common/BaseLogicLib/BaseLogic.h"
#include "../../Common/JsonLib/Json.h"

// Big chips

#include "../../Breaks6502/M6502Core/core.h"
#include "../../BreaksAPU/APUSim/apu.h"
#include "../../BreaksPPU/PPUSim/ppu.h"

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

// Cartridge Slot

#include "AbstractCartridge.h"
#include "CartridgeFactory.h"

// Mappers

#include "../../Mappers/NES_Header.h"
#include "../../Mappers/NROM.h"
#include "../../Mappers/UNROM.h"

// Boards

#include "SignalDefs.h"
#include "AbstractBoard.h"
#include "BogusBoard.h"
#include "NESBoard.h"
#include "FamicomBoard.h"
#include "BoardFactory.h"
#include "APUPlayerBoard.h"
#include "NSFMapper.h"
#include "NSFPlayerBoard.h"
#include "PPUPlayerBoard.h"

// To C#

#include "CoreApi.h"
