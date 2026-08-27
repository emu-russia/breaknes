// CartridgeLoader - creates a cartridge from a .nes image using the nescartdb
// identification (PRG/CHR CRC32 -> board type -> board JSON -> Pcb).

#pragma once

#include <string>
#include <cstdint>
#include <cstddef>

#include "../Mappers/AbstractCartridge.h"

namespace CartPcb
{
	/// <summary>
	/// Set the Nescartdb data directory (contains index.json and boards/).
	/// The default is "Nescartdb" relative to the working directory.
	/// </summary>
	void SetNescartdbDir(const char* dir);

	const std::string& GetNescartdbDir();

	/// <summary>
	/// Set a user directory with custom board JSONs (JSONES subset).
	/// May be empty. User boards override the built-in definitions.
	/// </summary>
	void SetUserBoardsDir(const char* dir);

	const std::string& GetUserBoardsDir();

	/// <summary>
	/// Create a cartridge from a .nes image using the CartPcb path:
	/// CRC32 of the PRG/CHR dumps -> nescartdb board type -> board JSON -> Pcb.
	/// Returns nullptr if identification or board loading fails (the caller
	/// falls back to the legacy iNES mapper path).
	/// </summary>
	Mappers::AbstractCartridge* CreateFromNesImage(
		Mappers::ConnectorType p1,
		uint8_t* nesImage, size_t nesImageSize);
}
