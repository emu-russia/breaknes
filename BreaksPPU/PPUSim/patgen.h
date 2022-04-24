// Pattern Address Generator

#pragma once

namespace PPUSim
{
	class PatBitInv
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState n_PCLK, BaseLogic::TriState O, BaseLogic::TriState INV, BaseLogic::TriState val_in,
			BaseLogic::TriState & val_out);
	};

	class PatBit
	{
		BaseLogic::DLatch pdin_latch;
		BaseLogic::DLatch pdout_latch;
		BaseLogic::DLatch ob_latch;
		BaseLogic::DLatch padx_latch;

	public:
		void sim(BaseLogic::TriState n_PCLK, BaseLogic::TriState O, BaseLogic::TriState val_OB, BaseLogic::TriState val_PD, BaseLogic::TriState PAR_O,
			BaseLogic::TriState & PADx);
	};

	class PATGen
	{
		PPU* ppu = nullptr;

		PatBitInv inv_bits[4]{};
		PatBit bits[7]{};

		BaseLogic::DLatch fnt_latch;
		BaseLogic::DLatch ob0_latch;
		BaseLogic::DLatch pad12_latch;

		BaseLogic::FF VINV_FF;

		BaseLogic::DLatch pdin_latch;
		BaseLogic::DLatch pdout_latch;
		BaseLogic::DLatch ob_latch;
		BaseLogic::DLatch pad4_latch;

		BaseLogic::DLatch pad0_latch;
		BaseLogic::DLatch pad1_latch;
		BaseLogic::DLatch pad2_latch;

		BaseLogic::TriState O = BaseLogic::TriState::X;
		BaseLogic::TriState VINV = BaseLogic::TriState::X;
		BaseLogic::TriState inv_bits_out[4]{};

		void sim_Control();
		void sim_VInv();
		void sim_PatBitsInv();
		void sim_PatBit4();
		void sim_PatBits();

	public:
		PATGen(PPU* parent);
		~PATGen();

		void sim();
	};
}
