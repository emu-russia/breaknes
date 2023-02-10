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

		if (bank_switch_enabled)
		{
			if ((addr & ~7) == BankRegBase)
			{
				sim_BankRegs(RnW, addr & 7, data);
			}
			else
			{
				sim_AccessBanked(RnW, addr, data);
			}
		}
		else
		{
			sim_AccessNotBanked(RnW, addr, data);
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

		ram_size = RoundUpPage(data_size);
		ram = new uint8_t[ram_size];
		memset(ram, 0xff, ram_size);
		memcpy(ram, data, data_size);
		load_addr = load_address;
	}

	void BankedSRAM::sim_BankRegs(TriState RnW, int reg_id, uint8_t* data)
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

	void BankedSRAM::sim_AccessNotBanked(TriState RnW, uint16_t addr, uint8_t* data)
	{
		if (addr < load_addr)
			return;

		int ofs = addr - load_addr;

		if (RnW == TriState::One)
		{
			*data = ram[ofs];
		}
		else
		{
			ram[ofs] = *data;
		}
	}

	void BankedSRAM::sim_AccessBanked(TriState RnW, uint16_t addr, uint8_t* data)
	{
		if (addr < 0x8000)
			return;

		// wtf?
		// "take the logical AND of the load address with $0FFF, and the result specifies the number of bytes of padding at the start of the ROM image."

		int bank = (addr >> 12) - 8;
		int mapped = bank_regs[bank];
		int ofs = mapped * 0x1000 + (addr & 0xFFF);

		if (RnW == TriState::One)
		{
			*data = ram[ofs];
		}
		else
		{
			ram[ofs] = *data;
		}
	}

	int BankedSRAM::RoundUpPage(int size)
	{
		int page_size = 0x1000;
		return ((((int)(size)) + page_size - 1) & (~(page_size - 1)));
	}
}
