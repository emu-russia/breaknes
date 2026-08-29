// Breaknes logging facility.
//
// The logging is controlled at two levels:
//
//  1. Compile time. The logging code is compiled in only when BREAKNES_LOG is
//     non-zero. By default it follows the build configuration: 1 in _DEBUG
//     builds and 0 in Release (NDEBUG), so Release does not contain the extra
//     logging code at all. Each subsystem can be overridden individually with
//     BREAKNES_LOG_<SUBSYSTEM> - e.g. a Release build can still keep the
//     M6502 core logs by defining BREAKNES_LOG_CORE=1.
//
//  2. Run time. The global Log manager keeps:
//       - the master "enabled" switch,
//       - the source mask (one bit per log source),
//       - a category mask per source (one bit per log category of that source),
//       - the output sinks (log file and/or stdout).
//     The macros below check the masks before evaluating the arguments, so
//     disabled logging costs a single branch.
//
// The sources are identified by the Log::Source enum (Common). The categories
// of each source are owned by the subsystem (chip) itself: every subsystem
// defines its own category enum in its header and exposes a public method to
// configure its logging (e.g. M6502Core::M6502::SetLogMask). BreaksCore pulls
// these definitions from the components (Component -> BreaksCore direction) and
// registers them for the user interface with Log::RegisterSource.

#pragma once

#include <cstdint>
#include <cstddef>

// ---------------------------------------------------------------------------
// Compile-time control
// ---------------------------------------------------------------------------

#ifndef BREAKNES_LOG

#if defined(_DEBUG)
#define BREAKNES_LOG 1
#else
#define BREAKNES_LOG 0
#endif

#endif // BREAKNES_LOG

// Per-subsystem overrides. Define BREAKNES_LOG_<SUBSYSTEM> to 1 to keep the
// logging code of one subsystem even in a build where BREAKNES_LOG is 0 (and
// to 0 to strip it where BREAKNES_LOG is 1).

#ifndef BREAKNES_LOG_CORE
#define BREAKNES_LOG_CORE BREAKNES_LOG
#endif

#ifndef BREAKNES_LOG_APU
#define BREAKNES_LOG_APU BREAKNES_LOG
#endif

#ifndef BREAKNES_LOG_PPU
#define BREAKNES_LOG_PPU BREAKNES_LOG
#endif

#ifndef BREAKNES_LOG_MMC1
#define BREAKNES_LOG_MMC1 BREAKNES_LOG
#endif

#ifndef BREAKNES_LOG_CARTPCB
#define BREAKNES_LOG_CARTPCB BREAKNES_LOG
#endif

#ifndef BREAKNES_LOG_BOARD
#define BREAKNES_LOG_BOARD BREAKNES_LOG
#endif

#ifndef BREAKNES_LOG_IO
#define BREAKNES_LOG_IO BREAKNES_LOG
#endif

namespace Log
{
	/// <summary>
	/// Log sources. One bit of the source mask controls one source.
	/// </summary>
	enum Source : uint32_t
	{
		Source_Core = 0,		// M6502Core (6502 CPU core)
		Source_APU,				// APUSim (RP2A03 and clones)
		Source_PPU,				// PPUSim (RP2C02 and clones)
		Source_MMC1,			// Chips/MMC1
		Source_CartPcb,			// CartPcb (cartridge PCBs)
		Source_Board,			// Breaknes boards (BreaksCore)
		Source_IO,				// IO subsystem (controllers etc.)
		Source_Count,
	};

	/// <summary>
	/// One log category of a source: a bit in the category mask plus a display name.
	/// The category sets are owned by the subsystems, not by this manager.
	/// </summary>
	struct LogCategoryDesc
	{
		uint64_t bit;
		const char* name;
	};

	// ---- Runtime configuration ----

	/// <summary>
	/// Master runtime switch. When disabled, no log lines are emitted
	/// (the source/category masks are ignored).
	/// </summary>
	void SetEnabled(bool enabled);

	/// <summary>
	/// true: logging is enabled at runtime (the compile-time switch still applies).
	/// </summary>
	bool IsEnabled();

	/// <summary>
	/// Enable/disable log sources by a bitmask (bit `n` = Source value `n`).
	/// </summary>
	void SetSourceMask(uint64_t mask);

	/// <summary>
	/// Get the current log source mask.
	/// </summary>
	uint64_t GetSourceMask();

	/// <summary>
	/// Set the category mask of one source. Each bit of the mask controls one
	/// category of that source.
	/// </summary>
	void SetCategoryMask(Source src, uint64_t mask);

	/// <summary>
	/// Get the current category mask of one source.
	/// </summary>
	uint64_t GetCategoryMask(Source src);

	/// <summary>
	/// Route the log to the given file ("w", truncated on open).
	/// An empty/null path closes the log file.
	/// </summary>
	void SetOutputFile(const char* path);

	/// <summary>
	/// Route the log to stdout.
	/// </summary>
	void SetOutputStdout(bool enable);

	// ---- Writing ----

	/// <summary>
	/// Fast check used by the log macros: true only if the source and the
	/// category are enabled. Safe to call on the hot path.
	/// </summary>
	bool IsCategoryEnabled(Source src, uint64_t cat);

	/// <summary>
	/// Write one log line, prefixed with "[Source][Category]".
	/// </summary>
	void Write(Source src, uint64_t cat, const char* fmt, ...);

	// ---- Registration / introspection (for the user interface) ----

	/// <summary>
	/// Register the display name and the category list of a source. Idempotent -
	/// call it once before the sources are queried (BreaksCore does it when a
	/// board is created).
	/// </summary>
	void RegisterSource(Source src, const char* name, const LogCategoryDesc* cats, size_t count);

	size_t GetSourceCount();

	/// <summary>
	/// Display name of the source ("?" while unregistered).
	/// </summary>
	const char* GetSourceName(Source src);

	size_t GetCategoryCount(Source src);

	/// <summary>
	/// The category bit of the `index`-th category of the source (0 when out of range).
	/// </summary>
	uint64_t GetCategoryBit(Source src, size_t index);

	/// <summary>
	/// Display name of the `index`-th category of the source ("?" when out of range).
	/// </summary>
	const char* GetCategoryName(Source src, size_t index);

	/// <summary>
	/// Display name of the category with the given bit ("?" when unknown).
	/// </summary>
	const char* GetCategoryNameByBit(Source src, uint64_t bit);
}

// ---------------------------------------------------------------------------
// Logging macros. One macro per subsystem, gated at compile time.
// Each macro takes the category bit first and then a printf-style format:
//
//     LOG_CORE(M6502Core::Cat_Events, "RESET asserted");
//     LOG_APU(APUSim::Cat_Regs, "Write $%04X = %02X", 0x4011, data);
//
// When the subsystem logging is compiled out, the macro expands to nothing and
// the arguments are not evaluated.
// ---------------------------------------------------------------------------

#if BREAKNES_LOG_CORE
#define LOG_CORE(cat, ...) do { if (Log::IsCategoryEnabled(Log::Source_Core, (uint64_t)(cat))) Log::Write(Log::Source_Core, (uint64_t)(cat), __VA_ARGS__); } while (0)
#else
#define LOG_CORE(cat, ...) ((void)0)
#endif

#if BREAKNES_LOG_APU
#define LOG_APU(cat, ...) do { if (Log::IsCategoryEnabled(Log::Source_APU, (uint64_t)(cat))) Log::Write(Log::Source_APU, (uint64_t)(cat), __VA_ARGS__); } while (0)
#else
#define LOG_APU(cat, ...) ((void)0)
#endif

#if BREAKNES_LOG_PPU
#define LOG_PPU(cat, ...) do { if (Log::IsCategoryEnabled(Log::Source_PPU, (uint64_t)(cat))) Log::Write(Log::Source_PPU, (uint64_t)(cat), __VA_ARGS__); } while (0)
#else
#define LOG_PPU(cat, ...) ((void)0)
#endif

#if BREAKNES_LOG_MMC1
#define LOG_MMC1(cat, ...) do { if (Log::IsCategoryEnabled(Log::Source_MMC1, (uint64_t)(cat))) Log::Write(Log::Source_MMC1, (uint64_t)(cat), __VA_ARGS__); } while (0)
#else
#define LOG_MMC1(cat, ...) ((void)0)
#endif

#if BREAKNES_LOG_CARTPCB
#define LOG_CART(cat, ...) do { if (Log::IsCategoryEnabled(Log::Source_CartPcb, (uint64_t)(cat))) Log::Write(Log::Source_CartPcb, (uint64_t)(cat), __VA_ARGS__); } while (0)
#else
#define LOG_CART(cat, ...) ((void)0)
#endif

#if BREAKNES_LOG_BOARD
#define LOG_BOARD(cat, ...) do { if (Log::IsCategoryEnabled(Log::Source_Board, (uint64_t)(cat))) Log::Write(Log::Source_Board, (uint64_t)(cat), __VA_ARGS__); } while (0)
#else
#define LOG_BOARD(cat, ...) ((void)0)
#endif

#if BREAKNES_LOG_IO
#define LOG_IO(cat, ...) do { if (Log::IsCategoryEnabled(Log::Source_IO, (uint64_t)(cat))) Log::Write(Log::Source_IO, (uint64_t)(cat), __VA_ARGS__); } while (0)
#else
#define LOG_IO(cat, ...) ((void)0)
#endif
