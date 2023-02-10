// A special type of memory with bank switching logic inside.

// "SRAM" means that the memory can be read and written from the simulator side. But in essence it is a "smart" ROM with a loaded NSF Data image.

#include "pch.h"

using namespace BaseLogic;

namespace NSFPlayer
{
	BankedSRAM::BankedSRAM()
	{
	}

	BankedSRAM::~BankedSRAM()
	{
		if (ram != nullptr)
		{
			delete[] ram;
		}
	}

	void BankedSRAM::sim(TriState RnW, TriState CS, uint16_t addr, uint8_t* data)
	{
		if (CS == TriState::Zero)
			return;

		if ((addr & ~7) == BankRegBase && bank_switch_enabled)
		{
			do_banking(RnW, addr & 7, data);
		}
		else
		{

		}
	}

	size_t BankedSRAM::Dbg_GetSize()
	{
		return ram_size;
	}

	uint8_t BankedSRAM::Dbg_ReadByte(size_t addr)
	{
		if (addr < ram_size)
		{
			return ram[addr];
		}
		else
		{
			return 0xff;
		}
	}

	void BankedSRAM::Dbg_WriteByte(size_t addr, uint8_t data)
	{
		if (addr < ram_size)
		{
			ram[addr] = data;
		}
	}

	uint8_t BankedSRAM::GetBankReg(int n)
	{
		return bank_regs[n & 7];
	}

	void BankedSRAM::SetBankReg(int n, uint8_t val)
	{
		bank_regs[n & 7] = val;
	}

	void BankedSRAM::LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
	{
		if (ram)
		{
			delete[] ram;
			ram = nullptr;
		}

		ram_size = data_size;
		ram = new uint8_t[ram_size];
		memcpy(ram, data, data_size);
		load_addr = load_address;
	}

	void BankedSRAM::do_banking(TriState RnW, int reg_id, uint8_t* data)
	{
		if (RnW == TriState::One)
		{
			*data = bank_regs[reg_id & 7];
		}
		else
		{
			bank_regs[reg_id & 7] = *data;
		}
	}

	void BankedSRAM::EnableNSFBanking(bool enable)
	{
		bank_switch_enabled = enable;
	}
}
