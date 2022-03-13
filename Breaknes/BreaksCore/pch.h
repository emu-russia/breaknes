#pragma once

#include <iostream>
#include <string>
#include <list>
#include <Windows.h>

#include "../BaseLogicLib/BaseLogic.h"

#include "../../Breaks6502/M6502Core/core.h"
#include "../../BreaksAPU/APUSim/apu.h"
#include "../../BreaksPPU/PPUSim/ppu.h"

#include "LS139.h"
#include "LS368.h"
#include "LS373.h"
#include "SRAM.h"

#include "AbstractCartridge.h"
#include "CartridgeFactory.h"

#include "AbstractBoard.h"
#include "NES.h"
#include "Famicom.h"
#include "BoardFactory.h"

#include "UnitTest.h"

#include "CoreApi.h"
#include "Core.h"

extern Breaknes::Core core;
