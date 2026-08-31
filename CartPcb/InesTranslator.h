// InesTranslator - fallback identification of "wild" iNES dumps (issue #514).
//
// When the PRG/CHR CRC32s of a dump are not found in nescartdb (homebrew and
// other undocumented dumps), the board type is inferred from the iNES header:
// the mapper number and the PRG/CHR sizes. This is a translation, not an
// identification: the resulting board type is a best-effort match for the
// header's intent, and the dump runs through the same CartPcb board path as a
// database-identified cartridge.

#pragma once

#include <cstddef>

#include "NesHeader.h"
#include "NesCartDb.h"

namespace CartPcb
{
	/// <summary>
	/// Translate a "wild" iNES dump to a board type when its PRG/CHR CRCs are
	/// not in nescartdb. The board is inferred from the iNES mapper number and
	/// the PRG/CHR sizes (see the mapper table in InesTranslator.cpp).
	/// Returns false when the dump cannot be translated (unknown mapper).
	/// </summary>
	/// <param name="head">The .nes header.</param>
	/// <param name="prgSize">PRG dump size in bytes (without the trainer).</param>
	/// <param name="chrSize">CHR dump size in bytes (0 = CHR-RAM).</param>
	/// <param name="out">Receives the translated board reference on success.</param>
	bool TryTranslateInesHeader(const NESHeader* head, size_t prgSize, size_t chrSize, BoardRef& out);
}
