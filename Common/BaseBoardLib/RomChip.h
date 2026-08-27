// A JEDEC-style ROM chip.
//
// Most NES cartridge ROMs are JEDEC-standard devices or similar (e.g. the 27xx
// EPROM family): pins A0..An, D0..D7, /CE (Chip Enable), /OE (Output Enable).
// A ROM is read-only: it has no /WE pin and ignores writes.
//
// The simulation follows the same "chip" style as SRAM (see SRAM.h): stick
// signals in, and if the chip is selected and output-enabled, it drives the
// data bus.

#pragma once

namespace BaseBoard
{
	class RomChip
	{
		size_t memSize = 0;
		uint8_t* mem = nullptr;
		bool do_trace = false;
		char rom_name[0x100]{};

	public:
		/// <summary>
		/// Create a ROM chip with 2^bits bytes of storage.
		/// </summary>
		/// <param name="entity">Chip name for tracing/debugging (e.g. "PRG", "CHR").</param>
		RomChip(const char* entity, size_t bits, bool trace = false);
		~RomChip();

		/// <summary>
		/// Load the ROM image (e.g. the PRG or CHR dump). The image is copied into
		/// the chip; if it is larger than the chip, the excess is ignored.
		/// </summary>
		void LoadImage(const uint8_t* image, size_t imageSize);

		/// <summary>
		/// Simulate the chip. JEDEC-style read: /CE == 0 && /OE == 0 -> drive the data bus.
		/// </summary>
		/// <param name="n_CE">input: 0: Chip Enable (JEDEC /CE)</param>
		/// <param name="n_OE">input: 0: Output Enable (JEDEC /OE)</param>
		/// <param name="addr">input: Address</param>
		/// <param name="data">inOut: Data Bus</param>
		/// <param name="dz">inOut: dz ("Data Z") determines whether the data bus is floating. dz = true means the bus is "floating"; a successful read sets it to false.</param>
		void sim(BaseLogic::TriState n_CE, BaseLogic::TriState n_OE, uint32_t* addr, uint8_t* data, bool& dz);

		size_t Dbg_GetSize();

		uint8_t Dbg_ReadByte(size_t addr);

		/// <summary>
		/// Debug-only write (e.g. loading images into CHR-RAM style regions or
		/// patching ROM from the debugger).
		/// </summary>
		void Dbg_WriteByte(size_t addr, uint8_t data);
	};
}
