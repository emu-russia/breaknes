#pragma once

namespace M6502Core
{
	union FlagsControl_TempWire
	{
		struct
		{
			unsigned n_POUT : 1;
			unsigned n_ARIT : 1;
			unsigned n_PIN : 1;
			unsigned ZTST : 1;
			unsigned SR : 1;
		};
		uint8_t bits;
	};

	class FlagsControl
	{
		friend M6502;
		friend RandomLogic;
		friend M6502CoreUnitTest::PackedProof;
		BaseLogic::DLatch pdb_latch{};
		BaseLogic::DLatch iri_latch{};
		BaseLogic::DLatch irc_latch{};
		BaseLogic::DLatch ird_latch{};
		BaseLogic::DLatch zv_latch{};
		BaseLogic::DLatch acrc_latch{};
		BaseLogic::DLatch dbz_latch{};
		BaseLogic::DLatch dbn_latch{};
		BaseLogic::DLatch dbc_latch{};
		BaseLogic::DLatch pin_latch{};
		BaseLogic::DLatch bit_latch{};

		M6502* core = nullptr;

		FlagsControl_TempWire* temp_tab = nullptr;
		RegsControl_TempWire prev_temp;

		FlagsControl_TempWire PreCalc(uint8_t ir, bool n_T0, bool n_T1X, bool n_T2, bool n_T3, bool n_T4, bool n_T5, bool T5, bool T6);

		// Packed: b1/b2 are the PHI1/PHI2 command-latch banks (random_logic.h); the reference form ignores them.
		template<bool Packed, int Phase = 0> void simImpl(uint64_t& command_word, uint64_t& b1, uint64_t& b2);

	public:

		FlagsControl(M6502* parent);
		~FlagsControl() { delete[] temp_tab; }

		void sim();
	};
}
