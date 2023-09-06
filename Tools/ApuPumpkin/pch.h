#pragma once

#define FAST_APU 0

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <list>
#include <string>
#include <cassert>
#include <Windows.h>

#include "../../Common/BaseLogicLib/BaseLogic.h"
#include "../../Chips/M6502Core/core.h"

#if FAST_APU
#include "../../Chips/FastAPU/apu.h"
#else
#include "../../Chips/APUSim/apu.h"
#endif
