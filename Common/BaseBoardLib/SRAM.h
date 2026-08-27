// A simulator of typical static RAM chips from the 80s and 90s (e.g. HM6116,
// 6264, 62256 and similar): standard pin functions /CS (Chip Select), /WE
// (Write Enable), /OE (Output Enable), A0..An, D0..D7.
//
// On power-up all memory cells read as 0 (matches the typical power-up state
// of these chips). The chip is asynchronous: an access is performed while the
// control pins are in the corresponding state, with no clock.
//
// Reference: https://github.com/emu-russia/breaks/blob/master/Docs/Famicom/HM6116_SRAM.pdf

#pragma once

namespace BaseBoard
{
	class SRAM
	{
		size_t memSize = 0;
		uint8_t* mem = nullptr;
		bool do_trace = false;
		char sram_name[0x100]{};

	public:
		/// <summary>
		/// Create a static RAM chip with 2^bits bytes of storage.
		/// </summary>
		/// <param name="entity">Chip name for tracing/debugging (e.g. "WRAM", "VRAM").</param>
		SRAM(const char *entity, size_t bits, bool trace=false);
		~SRAM();

		/// <summary>
		/// Simulate a typical static RAM chip from the 80s / 90s.
		/// </summary>
		/// <param name="n_CS">input: 0: Chip Select</param>
		/// <param name="n_WE">input: 0: Write Enable (Write Mode). Has a higher priority than `/OE` (see datasheet).</param>
		/// <param name="n_OE">input: 0: Output Enable (Read Mode)</param>
		/// <param name="addr">input: Address</param>
		/// <param name="data">inOut: Data Bus</param>
		/// <param name="dz">inOut: dz ("Data Z") determines whether the data bus to which the SRAM is attached is dirty or not. This is analogous to the z state of the bus (in Verilog terms). dz = true means that the bus is "floating".</param>
		void sim(BaseLogic::TriState n_CS, BaseLogic::TriState n_WE, BaseLogic::TriState n_OE, uint32_t *addr, uint8_t *data, bool& dz);

		size_t Dbg_GetSize();

		uint8_t Dbg_ReadByte(size_t addr);

		void Dbg_WriteByte(size_t addr, uint8_t data);
	};
}
