#pragma once

namespace M6502Core
{
	class BusControl
	{
		friend M6502;
		friend RandomLogic;
		BaseLogic::DLatch z_adh0_latch{};
		BaseLogic::DLatch z_adh17_latch{};
		BaseLogic::DLatch sb_ac_latch{};
		BaseLogic::DLatch adl_abl_latch{};
		BaseLogic::DLatch ac_sb_latch{};
		BaseLogic::DLatch sb_db_latch{};
		BaseLogic::DLatch ac_db_latch{};
		BaseLogic::DLatch sb_adh_latch{};
		BaseLogic::DLatch adh_abh_latch{};
		BaseLogic::DLatch dl_adh_latch{};
		BaseLogic::DLatch dl_adl_latch{};
		BaseLogic::DLatch dl_db_latch{};
		BaseLogic::DLatch nready_latch{};

		M6502* core = nullptr;

		// Packed: b1/b2 are the PHI1/PHI2 command-latch banks (random_logic.h); the reference form ignores them.
		template<bool Packed, int Phase = 0> void simImpl(uint64_t& command_word, uint64_t& b1, uint64_t& b2);

	public:

		BusControl(M6502* parent) { core = parent; }

		void sim();
	};
}
