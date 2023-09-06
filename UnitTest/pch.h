#pragma once

#include <cstdint>
#include <cstdio>
#include <list>
#include <string>
#include <cassert>
#include <Windows.h>

#include "../Common/BaseLogicLib/BaseLogic.h"

#include "../Chips/M6502Core/core.h"
#include "../Chips/APUSim/apu.h"
#include "../Chips/PPUSim/ppu.h"

#include "../IO/CD4021.h"

#include "../Common/JsonLib/Json.h"
#include "EventLog.h"

#include "CoreUnitTest.h"
#include "APUTest.h"
#include "PPUTest.h"
#include "CD4021UnitTest.h"

#include "CppUnitTest.h"
