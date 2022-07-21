// Generic SRAM

#pragma once

namespace BaseBoard
{
	class SRAM
	{
		size_t memSize = 0;
		uint8_t* mem = nullptr;

	public:
		SRAM(size_t bits);
		~SRAM();

		/// <summary>
		/// Simulate a typical SRAM chip from the 80s / 90s.
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
