#pragma once

#define FAST_APU 1

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <list>
#include <string>
#include <cassert>
#include <Windows.h>

#include "../../Common/BaseLogicLib/BaseLogic.h"
#include "../../Breaks6502/M6502Core/core.h"

#if FAST_APU
#include "../../BreaksAPU/FastAPU/apu.h"
#else
#include "../../BreaksAPU/APUSim/apu.h"
#endif
