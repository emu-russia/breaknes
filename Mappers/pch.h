#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <Windows.h>

#include "../Common/BaseLogicLib/BaseLogic.h"
#include "../Breaknes/BreaksCore/DebugHub.h"
#include "../Breaks6502/M6502Core/core.h"
#include "../BreaksAPU/APUSim/apu.h"
#include "../Breaknes/BreaksCore/AbstractCartridge.h"

#include "NES_Header.h"
#include "NROM.h"
