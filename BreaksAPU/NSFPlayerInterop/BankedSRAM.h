// A special type of memory with bank switching logic inside.

#pragma once

namespace NSFPlayer
{
	class BankedSRAM
	{
		uint8_t bank_regs[8]{};

		uint8_t* ram = nullptr;
		size_t ram_size = 0;

		uint16_t load_addr = 0;
		bool bank_switch_enabled = false;

		void sim_BankRegs (BaseLogic::TriState RnW, int reg_id, uint8_t* data);
		void sim_AccessNotBanked(BaseLogic::TriState RnW, uint16_t addr, uint8_t* data);
		void sim_AccessBanked(BaseLogic::TriState RnW, uint16_t addr, uint8_t* data);

		const uint16_t BankRegBase = 0x5ff8;

		int RoundUpPage(int size);

	public:
		BankedSRAM();
		~BankedSRAM();

		void sim(BaseLogic::TriState RnW, BaseLogic::TriState CS, uint16_t addr, uint8_t* data);

		uint8_t GetBankReg(int n);
		void SetBankReg(int n, uint8_t val);

		size_t Dbg_GetSize();
		uint8_t Dbg_ReadByte(size_t addr);
		void Dbg_WriteByte(size_t addr, uint8_t data);

		void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address);

		void EnableNSFBanking(bool enable);
	};
}
