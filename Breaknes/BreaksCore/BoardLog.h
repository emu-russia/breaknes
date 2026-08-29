// Log categories of the Breaknes board (BreaksCore). Kept in a small
// dependency-free header so that files compiled in multiple contexts
// (BreaksCore, UnitTest) can use them without pulling in the board headers.

#pragma once

#include "../../Common/BaseLogicLib/Log.h"

namespace Breaknes
{
	/// <summary>
	/// Log categories of the board (one bit per category, owned by this component).
	/// </summary>
	enum LogCategory : uint64_t
	{
		Cat_Events = 1ULL << 0,		// board lifecycle events (create/destroy, cart insert/eject, reset)
		Cat_Bus = 1ULL << 1,		// CPU bus cycles
	};

	/// <summary>
	/// The category list of the board, used by BreaksCore to register the source
	/// for the user interface (definitions flow Component -> BreaksCore).
	/// </summary>
	const Log::LogCategoryDesc* GetLogCategories(size_t& count);
}
