// A special type of memory with bank switching logic inside.

#pragma once

namespace NSFPlayer
{
	class BankedSRAM
	{
		uint8_t bank_regs[8]{};

	public:
		BankedSRAM();
		~BankedSRAM();

		void sim(BaseLogic::TriState RnW, uint16_t addr, uint8_t* data);

		uint8_t GetBankReg(int n);
		void SetBankReg(int n, uint8_t val);

		size_t Dbg_GetSize();
		uint8_t Dbg_ReadByte(size_t addr);
		void Dbg_WriteByte(size_t addr, uint8_t data);
	};
}
