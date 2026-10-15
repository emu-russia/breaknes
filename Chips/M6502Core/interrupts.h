#pragma once

namespace M6502Core
{
	class BRKProcessing
	{
		friend M6502;
		friend RandomLogic;
		BaseLogic::DLatch brk5_latch{};
		BaseLogic::DLatch brk6_latch1{};
		BaseLogic::DLatch brk6_latch2{};
		BaseLogic::DLatch res_latch1{};
		BaseLogic::DLatch res_latch2{};
		BaseLogic::DLatch brk6e_latch{};
		BaseLogic::DLatch brk7_latch{};
		BaseLogic::DLatch nmip_latch{};
		BaseLogic::DLatch donmi_latch{};
		BaseLogic::DLatch ff1_latch{};
		BaseLogic::DLatch ff2_latch{};
		BaseLogic::DLatch delay_latch1{};
		BaseLogic::DLatch delay_latch2{};
		BaseLogic::DLatch b_latch1{};
		BaseLogic::DLatch b_latch2{};

		BaseLogic::DLatch zadl_latch[3]{};

		M6502* core = nullptr;

		template<bool Packed, int Phase = 0> void sim_BeforeRandomImpl();
		template<bool Packed, int Phase = 0> void sim_AfterRandomImpl();
		// ExactPacked, PHI1 second traversal: the part of sim_BeforeRandom that can differ from the
		// first traversal (see M6502::sim_Phi1TopSettle).
		void sim_SettlePhi1BeforeRandom();

	public:

		BRKProcessing(M6502* parent) { core = parent; }

		void sim_BeforeRandom();
		void sim_AfterRandom();

		BaseLogic::TriState getDORES();
		BaseLogic::TriState getB_OUT(BaseLogic::TriState BRK6E);
		BaseLogic::TriState getn_BRK6_LATCH2();
	};
}
