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

		void sim(BaseLogic::TriState n_CS, BaseLogic::TriState n_WE, BaseLogic::TriState n_OE, uint32_t *addr, uint8_t *data, bool& dz);

		size_t Dbg_GetSize();

		uint8_t Dbg_ReadByte(size_t addr);

		void Dbg_WriteByte(size_t addr, uint8_t data);
	};
}
