#pragma once

namespace M6502Core
{
	class ALU
	{
		uint8_t AI = 0;
		uint8_t BI = 0;
		uint8_t n_ADD = 0xff;
		uint8_t AC = 0;

		BaseLogic::DLatch BC7_latch{};
		BaseLogic::DLatch DC7_latch{};

		BaseLogic::DLatch daal_latch{};
		BaseLogic::DLatch daah_latch{};
		BaseLogic::DLatch dsal_latch{};
		BaseLogic::DLatch dsah_latch{};

		BaseLogic::DLatch DCLatch{};
		BaseLogic::DLatch ACLatch{};
		BaseLogic::DLatch AVRLatch{};

		bool BCD_Hack = false;		// BCD correction hack for NES/Famicom.

		M6502* core = nullptr;

	public:

		ALU(M6502* parent) { core = parent; }

		void sim_Load();

		void sim();

		void sim_HLE();

		void sim_StoreADD();

		void sim_StoreAC();

		void sim_BusMux();

		BaseLogic::TriState getACR();

		BaseLogic::TriState getAVR();
		void setAVR(BaseLogic::TriState);

		uint8_t getAI();
		uint8_t getBI();
		uint8_t getADD();
		uint8_t getAC();

		void setAI(uint8_t val);
		void setBI(uint8_t val);
		void setADD(uint8_t val);
		void setAC(uint8_t val);

		void SetBCDHack(bool enable) { BCD_Hack = enable; }
	};
}
