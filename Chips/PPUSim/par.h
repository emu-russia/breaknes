// Picture Address Register

#pragma once

namespace PPUSim
{
	class ParBitInv
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState n_PCLK, BaseLogic::TriState O, BaseLogic::TriState INV, BaseLogic::TriState val_in,
			BaseLogic::TriState& val_out);
	};

	class ParBit
	{
		BaseLogic::DLatch pdin_latch;
		BaseLogic::DLatch pdout_latch;
		BaseLogic::DLatch ob_latch;
		BaseLogic::DLatch padx_latch;

	public:
		void sim(BaseLogic::TriState n_PCLK, BaseLogic::TriState O, BaseLogic::TriState val_OB, BaseLogic::TriState val_PD, BaseLogic::TriState OBJ_READ,
			BaseLogic::TriState& PADx);
	};

	class PAR
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		ParBitInv inv_bits[4]{};
		ParBit bits[7]{};

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
		void sim_ParBitsInv();
		void sim_ParBit4();
		void sim_ParBits();

	public:
		PAR(PPU* parent);
		~PAR();

		void sim();
	};
}
