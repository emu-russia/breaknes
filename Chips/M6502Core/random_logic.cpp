#include "pch.h"
#ifdef FASTSIM_SKIP_ORACLE
#include <cstdio>
#include <cstdlib>
#include <cstring>
#endif

using namespace BaseLogic;

namespace M6502Core
{
	RandomLogic::RandomLogic(M6502* parent)
	{
		core = parent;
		regs_control = new RegsControl(core);
		alu_control = new ALUControl(core);
		pc_control = new PC_Control(core);
		bus_control = new BusControl(core);
		flags_control = new FlagsControl(core);
		flags = new Flags(core);
		branch_logic = new BranchLogic(core);
	}

	RandomLogic::~RandomLogic()
	{
		delete regs_control;
		delete alu_control;
		delete pc_control;
		delete bus_control;
		delete flags_control;
		delete flags;
		delete branch_logic;
	}

	void RandomLogic::sim()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked) sim_Impl<true>();
		else sim_Impl<false>();
	}

	template<bool Packed, int Phase> void RandomLogic::sim_Impl(bool replay)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
#if M6502_PACKED_SINGLE_COMMIT
		uint64_t command_word = core->cmd.raw;
#else
		uint64_t& command_word = core->cmd.raw;
#endif
		if constexpr (Packed)
		{
			if (replay)
			{
				// Second Top traversal of a call with a binary PHI0. Every input of the five command
				// evaluators is either frozen for the whole phase (PHI2-enabled latches in PHI1,
				// PHI1-enabled latches in PHI2, IR, the decoder row in PHI2) or settles in the first
				// traversal (n_ready, ACRL2, BRK6E, Z_ADL0, T0 in PHI2), and the few PHI1-settled
				// signals they do read (T0, T1, BRFW, flags, the row) only reach PHI2-enabled latches.
				// Their second evaluation therefore writes every latch, member and command bit with the
				// value it already holds, except AVR_V, which is the live decoder line d[112] and is
				// refreshed here from the current row (the row can change in PHI1's second traversal).
				// FASTSIM_SKIP_ORACLE builds re-run the evaluators and abort on any difference.
#ifdef FASTSIM_SKIP_ORACLE
				sim_ReplayOracle();
#else
				command_word = (command_word & ~(UINT64_C(1) << 57)) |
					(uint64_t(DecoderView<true>(core->decoder_out, core->packed_decoder)[112]) << 57);
#endif
				branch_logic->simImpl<Packed, Phase>();
				return;
			}
		}

		// The packed command-latch banks (random_logic.h) travel through the five evaluators as locals and
		// are stored back once; nothing between the evaluators reads them from the core.
		uint64_t b1 = core->cmd_latch1, b2 = core->cmd_latch2;

		// Register control

		regs_control->simImpl<Packed, Phase>(command_word, b1, b2);

		// ALU control

		alu_control->simImpl<Packed, Phase>(command_word, b1, b2);

		// Program counter (PC) control

		pc_control->simImpl<Packed, Phase>(command_word, b1, b2);

		// Bus control

		bus_control->simImpl<Packed, Phase>(command_word, b1, b2);

		// Flags control logic

		flags_control->simImpl<Packed, Phase>(command_word, b1, b2);

		if constexpr (Packed) { core->cmd_latch1 = b1; core->cmd_latch2 = b2; }

		// The processing of loading flags has moved to the bottom part.

		// Conditional branch logic

		branch_logic->simImpl<Packed, Phase>();
#if M6502_PACKED_SINGLE_COMMIT
		if constexpr (Packed) core->cmd.raw = command_word;
#endif
	}
	template void RandomLogic::sim_Impl<false, 0>(bool);
	template void RandomLogic::sim_Impl<true, 0>(bool);
	template void RandomLogic::sim_Impl<true, 1>(bool);
	template void RandomLogic::sim_Impl<true, 2>(bool);

#ifdef FASTSIM_SKIP_ORACLE
	// Re-run the five command evaluators that the replay traversal skipped and abort if any
	// latch, member, dispatcher wire or command bit (other than the refreshed AVR_V) differs.
	void RandomLogic::sim_ReplayOracle()
	{
		uint8_t regs_before[sizeof(RegsControl)], alu_before[sizeof(ALUControl)], pc_before[sizeof(PC_Control)];
		uint8_t bus_before[sizeof(BusControl)], flags_before[sizeof(FlagsControl)];
		std::memcpy(regs_before, static_cast<const void*>(regs_control), sizeof(regs_before));
		std::memcpy(alu_before, static_cast<const void*>(alu_control), sizeof(alu_before));
		std::memcpy(pc_before, static_cast<const void*>(pc_control), sizeof(pc_before));
		std::memcpy(bus_before, static_cast<const void*>(bus_control), sizeof(bus_before));
		std::memcpy(flags_before, static_cast<const void*>(flags_control), sizeof(flags_before));
		const TriState pc_db = core->wire.PC_DB, n_adl_pcl = core->wire.n_ADL_PCL;
		const uint64_t before = core->cmd.raw;
		const uint64_t latch1_before = core->cmd_latch1, latch2_before = core->cmd_latch2;
		const uint64_t expected = (before & ~(UINT64_C(1) << 57)) |
			(uint64_t(DecoderView<true>(core->decoder_out, core->packed_decoder)[112]) << 57);
		regs_control->simImpl<true>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
		alu_control->simImpl<true>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
		pc_control->simImpl<true>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
		bus_control->simImpl<true>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
		flags_control->simImpl<true>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
		const bool same = latch1_before == core->cmd_latch1 && latch2_before == core->cmd_latch2 &&
			std::memcmp(regs_before, static_cast<const void*>(regs_control), sizeof(regs_before)) == 0 &&
			std::memcmp(alu_before, static_cast<const void*>(alu_control), sizeof(alu_before)) == 0 &&
			std::memcmp(pc_before, static_cast<const void*>(pc_control), sizeof(pc_before)) == 0 &&
			std::memcmp(bus_before, static_cast<const void*>(bus_control), sizeof(bus_before)) == 0 &&
			std::memcmp(flags_before, static_cast<const void*>(flags_control), sizeof(flags_before)) == 0 &&
			core->cmd.raw == expected && pc_db == core->wire.PC_DB && n_adl_pcl == core->wire.n_ADL_PCL;
		if (!same)
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: M6502 command-evaluator replay violated (PHI2 %d, cmd %016llx -> %016llx, expected %016llx, latches %016llx/%016llx -> %016llx/%016llx)\n",
				int(core->wire.PHI2), (unsigned long long)before, (unsigned long long)core->cmd.raw, (unsigned long long)expected,
				(unsigned long long)latch1_before, (unsigned long long)latch2_before, (unsigned long long)core->cmd_latch1, (unsigned long long)core->cmd_latch2);
			std::abort();
		}
	}
#endif

}
