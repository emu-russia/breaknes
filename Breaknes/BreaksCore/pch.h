#pragma once

#include <iostream>
#include <string>
#include <list>
#include <cassert>
#include <Windows.h>

#include "../../Common/BaseLogicLib/BaseLogic.h"
#include "../../Common/JsonLib/Json.h"

#include "../../Breaks6502/M6502Core/core.h"
#include "../../BreaksAPU/APUSim/apu.h"
#include "../../BreaksPPU/PPUSim/ppu.h"

namespace Breaknes
{
	class Core;
}

#include "../../Common/BaseBoardLib/LS139.h"
#include "../../Common/BaseBoardLib/LS368.h"
#include "../../Common/BaseBoardLib/LS373.h"
#include "../../Common/BaseBoardLib/SRAM.h"
#include "../../Common/DebugHub/DebugHub.h"

#include "AbstractCartridge.h"
#include "CartridgeFactory.h"

#include "AbstractBoard.h"
#include "BogusBoard.h"
#include "NES.h"
#include "Famicom.h"
#include "BoardFactory.h"

#include "CoreApi.h"
#include "Core.h"

extern Breaknes::Core core;
