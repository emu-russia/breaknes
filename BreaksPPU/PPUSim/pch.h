#pragma once

#include <cstdint>
#include <cassert>
#include <cstdio>
#include <random>
#include <algorithm>
#ifdef _WIN32
#include <intrin.h>
#endif
#include <cstddef>
#include <ctime>

#pragma warning(disable: 26812)		// warning C26812: The enum type 'BaseLogic::TriState' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).

#include "../../Common/BaseLogicLib/BaseLogic.h"

#include "ppu.h"
