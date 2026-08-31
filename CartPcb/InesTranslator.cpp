// InesTranslator - fallback identification of "wild" iNES dumps (issue #514).

#include "InesTranslator.h"

namespace CartPcb
{
	namespace
	{
		int GetMapperNumber(const NESHeader* head)
		{
			// iNES mapper number: low nibble of Flags_6 + high nibble of Flags_7.
			return (int)((head->Flags_6 >> 4) | ((head->Flags_7 >> 4) << 4));
		}
	}

	bool TryTranslateInesHeader(const NESHeader* head, size_t prgSize, size_t chrSize, BoardRef& out)
	{
		if (head == nullptr)
			return false;

		out = BoardRef();
		out.mapper = GetMapperNumber(head);

		// Mapper -> board type table. Each type must resolve to a board JSON
		// (Nescartdb/boards/): the built-in family definitions (nrom/unrom/
		// aorom/sgrom) or the fallback boards added for this path (cnrom/shrom).
		// Board JSONs adapt to the actual dump sizes (components without an
		// explicit `size` take the size of the corresponding image).
		switch (out.mapper)
		{
			case 0:
				// NROM: 16 KiB PRG -> NROM-128, 32 KiB PRG -> NROM-256.
				out.type = (prgSize <= 0x4000) ? "NES-NROM-128" : "NES-NROM-256";
				break;

			case 1:
				// MMC1: CHR-ROM -> the generic MMC1 + CHR-ROM wiring (sgrom.json);
				// CHR-RAM -> the generic MMC1 + CHR-RAM wiring (shrom.json).
				out.type = (chrSize != 0) ? "HVC-SGROM" : "HVC-SHROM";
				break;

			case 2:
				out.type = "NES-UNROM";
				break;

			case 3:
				out.type = "NES-CNROM";
				break;

			case 7:
				out.type = "NES-AOROM";
				break;

			default:
				// No translation for this mapper yet (e.g. MMC3 needs the MMC3
				// chip simulation first). The dump is reported as unsupported.
				return false;
		}

		// The scroll of scroll-jumper boards is applied by the cartridge loader
		// from the iNES mirroring bit (Flags6 bit 0) directly, so no scroll data
		// is stored here.
		return true;
	}
}
