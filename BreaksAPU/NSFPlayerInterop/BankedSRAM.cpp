// A special type of memory with bank switching logic inside.

#include "pch.h"

namespace NSFPlayer
{
	BankedSRAM::BankedSRAM()
	{
	}

	BankedSRAM::~BankedSRAM()
	{
	}

	void BankedSRAM::sim(BaseLogic::TriState RnW, uint16_t addr, uint8_t* data)
	{
	}

	size_t BankedSRAM::Dbg_GetSize()
	{
		return 0;
	}

	uint8_t BankedSRAM::Dbg_ReadByte(size_t addr)
	{
		return 0;
	}

	void BankedSRAM::Dbg_WriteByte(size_t addr, uint8_t data)
	{

	}

	uint8_t BankedSRAM::GetBankReg(int n)
	{
		return bank_regs[n & 7];
	}

	void BankedSRAM::SetBankReg(int n, uint8_t val)
	{
		bank_regs[n & 7] = val;
	}
}
