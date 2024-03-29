#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <cstddef>

#include "../Common/BaseLogicLib/BaseLogic.h"
#include "../Breaknes/BreaksCore/DebugHub.h"
#include "../Chips/M6502Core/core.h"
#include "../Chips/APUSim/apu.h"

#include "../Common/BaseBoardLib/LS32.h"
#include "../Common/BaseBoardLib/LS161.h"

#include "AbstractCartridge.h"
#include "NES_Header.h"
#include "NROM.h"
#include "UNROM.h"
#include "AOROM.h"
#include "MMC1.h"
#include "MMC1_Based.h"
// Add new mappers here
// ...
#include "CartridgeFactory.h"
