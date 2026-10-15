#pragma once

namespace M6502Core
{
	class PC_Control
	{
		friend M6502;
		friend RandomLogic;
		BaseLogic::DLatch pcl_db_latch1{};
		BaseLogic::DLatch pcl_db_latch2{};
		BaseLogic::DLatch pch_db_latch1{};
		BaseLogic::DLatch pch_db_latch2{};
		BaseLogic::DLatch nready_latch{};
		BaseLogic::DLatch pcl_adl_latch{};
		BaseLogic::DLatch pch_adh_latch{};
		BaseLogic::DLatch pcl_pcl_latch{};
		BaseLogic::DLatch adl_pcl_latch{};
		BaseLogic::DLatch adh_pch_latch{};
		BaseLogic::DLatch pch_pch_latch{};

		M6502* core = nullptr;

		// Packed: b1/b2 are the PHI1/PHI2 command-latch banks (random_logic.h); the reference form ignores them.
		template<bool Packed, int Phase = 0> void simImpl(uint64_t& command_word, uint64_t& b1, uint64_t& b2);

	public:

		PC_Control(M6502* parent) { core = parent; }

		void sim();
	};
}
