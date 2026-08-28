// CartridgeLoader - creates a cartridge from a .nes image using the nescartdb
// identification (PRG/CHR CRC32 -> board type -> board JSON -> Pcb).

#pragma once

#include <string>
#include <cstdint>
#include <cstddef>

#include "CartPcbPort.h"

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
	/// Force a board type for every cartridge (JSONES hook for custom PCBs that
	/// are not in the nescartdb index): the nescartdb identification is skipped
	/// and the board definition is loaded directly from the built-in or user
	/// board JSONs. Empty string (default) disables the override.
	/// </summary>
	void SetForcedBoardType(const char* type);

	const std::string& GetForcedBoardType();

	/// <summary>
	/// Create a cartridge from a .nes image using the CartPcb path:
	/// CRC32 of the PRG/CHR dumps -> nescartdb board type -> board JSON -> Pcb.
	/// Returns nullptr if identification or board loading fails.
	/// </summary>
	Cartridge* CreateFromNesImage(
		ConnectorType p1,
		uint8_t* nesImage, size_t nesImageSize);
}
