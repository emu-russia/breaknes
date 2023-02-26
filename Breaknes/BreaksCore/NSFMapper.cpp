// A special type of memory with bank switching logic inside.

// In essence it is a "smart" ROM with a loaded NSF Data image.

#include "pch.h"

using namespace BaseLogic;

namespace Breaknes
{
	NSFMapper::NSFMapper()
	{
	}

	NSFMapper::~NSFMapper()
	{
		if (ram != nullptr)
		{
			delete[] ram;
		}
	}

	void NSFMapper::sim(TriState RnW, TriState CS, uint16_t addr, uint8_t* data)
	{
		if (CS == TriState::Zero)
			return;

		//printf("NSF Data access: 0x%04X, read: %d\n", addr, RnW == TriState::One ? 1 : 0);

		if (faking_reset_fc)
		{
			if (addr == 0xfffc && RnW == TriState::One)
			{
				*data = (uint8_t)fake_reset;
				//faking_reset_fc = false;
				return;
			}
		}

		if (faking_reset_fd)
		{
			if (addr == 0xfffd && RnW == TriState::One)
			{
				*data = (uint8_t)(fake_reset >> 8);
				//faking_reset_fd = false;
				return;
			}
		}

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

	size_t NSFMapper::Dbg_GetSize()
	{
		return ram_size;
	}

	uint8_t NSFMapper::Dbg_ReadByte(size_t addr)
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

	void NSFMapper::Dbg_WriteByte(size_t addr, uint8_t data)
	{
		if (addr < ram_size)
		{
			ram[addr] = data;
		}
	}

	uint8_t NSFMapper::GetBankReg(int n)
	{
		return bank_regs[n & 7];
	}

	void NSFMapper::SetBankReg(int n, uint8_t val)
	{
		bank_regs[n & 7] = val;
	}

	void NSFMapper::LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
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

	void NSFMapper::sim_BankRegs(TriState RnW, int reg_id, uint8_t* data)
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

	void NSFMapper::EnableNSFBanking(bool enable)
	{
		bank_switch_enabled = enable;
	}

	void NSFMapper::sim_AccessNotBanked(TriState RnW, uint16_t addr, uint8_t* data)
	{
		if (addr < load_addr)
			return;

		int ofs = addr - load_addr;

		if (RnW == TriState::One)
		{
			*data = ram[ofs];
		}
	}

	void NSFMapper::sim_AccessBanked(TriState RnW, uint16_t addr, uint8_t* data)
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
	}

	size_t NSFMapper::RoundUpPage(size_t size)
	{
		const size_t page_size = 0x1000;
		return ((((size_t)(size)) + page_size - 1) & (~(page_size - 1)));
	}

	void NSFMapper::SetFakeResetVector(uint16_t addr)
	{
		fake_reset = addr;
		faking_reset_fc = true;
		faking_reset_fd = true;
	}
}
