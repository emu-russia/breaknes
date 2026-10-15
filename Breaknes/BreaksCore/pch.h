#pragma once

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <cassert>
#include <memory.h>
#include <cstddef>
#include <cstdint>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "../../Tools/Breakasm/asm.h"
#include "../../Tools/Breakasm/asmops.h"
#include "../../Tools/Breakasm/asmexpr.h"

// Very basic

#include "../../Common/BaseLogicLib/BaseLogic.h"
#include "../../Common/JsonLib/Json.h"

// Big chips

#define FAST_APU 1
#define FAST_PPU 1

#include "../../Chips/M6502Core/core.h"
#include "../../Chips/APUSim/apu.h"
#include "../../Chips/APUSimFast/apu.h"
#include "../../Chips/PPUSim/ppu.h"
#include "../../Chips/PPUSimFast/ppu.h"
#include "../../Chips/MMC1/MMC1.h"

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

// Cartridge (CartPcb: functional simulation of cartridge PCBs)

#include "../../CartPcb/CartPcb.h"

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
