// NesCartDb - cartridge identification by content.
//
// Maps the CRC32 of the PRG and CHR dumps to a board type using the converted
// nescartdb index (Nescartdb/index.json, see Nescartdb/Readme.md).

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace CartPcb
{
	struct BoardRef
	{
		std::string system;
		std::string type;
		std::string pcb;
		int mapper = -1;	// informational only; identification is by CRC, not by iNES mapper
		int padH = -1;		// mirroring solder pads (-1: unknown)
		int padV = -1;
	};

	class NesCartDb
	{
	public:
		/// <summary>
		/// Load the lookup index (index.json). Returns false on failure.
		/// </summary>
		bool Load(const std::string& indexPath);

		/// <summary>
		/// Find all boards whose PRG/CHR CRC32 pair matches the dumps.
		/// Several boards may share the same dump (same ROMs on different PCBs);
		/// all matches are returned.
		/// </summary>
		void FindBoards(uint32_t prgCrc, uint32_t chrCrc, std::vector<BoardRef>& out) const;

		bool IsLoaded() const { return loaded; }

		/// <summary>
		/// Standard CRC-32 (IEEE 802.3), the checksum used by nescartdb.
		/// </summary>
		static uint32_t Crc32(const uint8_t* data, size_t size);

	private:
		struct Record
		{
			uint32_t prgCrc = 0;
			uint32_t chrCrc = 0;
			BoardRef ref;
		};

		std::vector<Record> records;
		bool loaded = false;
	};
}
