#pragma once

#define FAST_PPU 0

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <list>
#include <string>
#include <cassert>
#include <Windows.h>

#include "../../Common/BaseLogicLib/BaseLogic.h"

#if FAST_PPU
#include "../../Chips/PPUSimFast/ppu.h"
#else
#include "../../Chips/PPUSim/ppu.h"
#endif
