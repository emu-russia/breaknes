#pragma once

namespace M6502Core
{
	class Dispatcher
	{
		friend M6502;
		friend RandomLogic;
		BaseLogic::DLatch acr_latch1{};
		BaseLogic::DLatch acr_latch2{};

		BaseLogic::DLatch t67_latch{};
		BaseLogic::DLatch t6_latch1{};
		BaseLogic::DLatch t6_latch2{};
		BaseLogic::DLatch t7_latch1{};
		BaseLogic::DLatch t7_latch2{};
		
		BaseLogic::DLatch tres2_latch{};
		BaseLogic::DLatch tresx_latch1{};
		BaseLogic::DLatch tresx_latch2{};

		BaseLogic::DLatch fetch_latch{};
		BaseLogic::DLatch wr_latch{};
		BaseLogic::DLatch ready_latch1{};
		BaseLogic::DLatch ready_latch2{};

		BaseLogic::DLatch ends_latch1{};
		BaseLogic::DLatch ends_latch2{};

		BaseLogic::DLatch nready_latch{};
		BaseLogic::DLatch step_latch1{};
		BaseLogic::DLatch step_latch2{};
		BaseLogic::DLatch t1_latch{};

		BaseLogic::DLatch comp_latch1{};
		BaseLogic::DLatch comp_latch2{};
		BaseLogic::DLatch comp_latch3{};

		BaseLogic::DLatch rdydelay_latch1{};
		BaseLogic::DLatch rdydelay_latch2{};

		BaseLogic::DLatch t0_latch{};
		BaseLogic::DLatch t1x_latch{};

		BaseLogic::DLatch br_latch1{};
		BaseLogic::DLatch br_latch2{};
		BaseLogic::DLatch ipc_latch1{};
		BaseLogic::DLatch ipc_latch2{};
		BaseLogic::DLatch ipc_latch3{};

		M6502* core = nullptr;

		template<int Phase> void sim_BeforeDecoderImpl();
		template<bool Packed, int Phase = 0> void sim_BeforeRandomLogicImpl();
		template<bool Packed, int Phase = 0> void sim_AfterRandomLogicImpl();
		// ExactPacked, PHI2 second traversal: the parts of the two blocks that can differ from the
		// first traversal (see M6502::sim_Phi2TopSettle).
		void sim_SettlePhi2BeforeDecoder();
		void sim_SettlePhi2AfterRandomLogic();
		// ExactPacked, PHI1 second traversal: the part of sim_BeforeDecoder that can differ from the
		// first traversal (see M6502::sim_Phi1TopSettle).
		void sim_SettlePhi1BeforeDecoder();

	public:

		Dispatcher(M6502* parent) { core = parent; }
		
		void sim_BeforeDecoder();

		void sim_BeforeRandomLogic();

		void sim_AfterRandomLogic();

		BaseLogic::TriState getTRES2();

		BaseLogic::TriState getT1();

		BaseLogic::TriState getSTOR(BaseLogic::TriState d[]);
	};
}
