#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	void BRKProcessing::sim_BeforeRandom()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked) sim_BeforeRandomImpl<true>();
		else sim_BeforeRandomImpl<false>();
	}

	template<bool Packed, int Phase> void BRKProcessing::sim_BeforeRandomImpl()
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		uint64_t commands = 0;
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState BRK5 = DecoderView<Packed>(core->decoder_out,core->packed_decoder)[22];
		TriState n_ready = core->wire.n_ready;
		TriState RESP = core->wire.RESP;
		TriState n_NMIP = core->wire.n_NMIP;

		// Interrupt cycle 6-7

		TriState BRK5_RDY = AND(BRK5, NOT(n_ready));
		P::Set2(brk5_latch, BRK5_RDY, PHI2);
		P::Set1(brk6_latch1, AND(NOT(brk5_latch.get()), NAND(n_ready, brk6_latch1.nget())), PHI1);
		P::Set2(brk6_latch2, brk6_latch1.nget(), PHI2);
		TriState BRK6E = NOR(brk6_latch2.nget(), n_ready);
		TriState BRK7 = NOR(brk6_latch1.nget(), BRK5_RDY);

		// Reset FF

		P::Set2(res_latch1, RESP, PHI2);
		P::Set1(res_latch2, NOR(NOR(res_latch1.get(), res_latch2.get()), BRK6E), PHI1);
		TriState res_out = NOR(res_latch1.get(), res_latch2.get());
		TriState DORES = NOT(res_out);

		// NMI Edge Detect

		P::Set1(brk6e_latch, BRK6E, PHI1);
		P::Set2(brk7_latch, BRK7, PHI2);
		P::Set1(nmip_latch, n_NMIP, PHI1);

		P::Set1(donmi_latch, NOR3(brk7_latch.nget(), n_NMIP, NOR(nmip_latch.get(), NOR(ff2_latch.get(), delay_latch2.get()))), PHI1);

		P::Set2(ff1_latch, NOR(donmi_latch.get(), NOR(ff1_latch.get(), brk6e_latch.get())), PHI2);
		TriState n_DONMI = NOR(donmi_latch.get(), NOR(ff1_latch.get(), brk6e_latch.get()));

		P::Set2(delay_latch1, n_DONMI, PHI2);
		P::Set1(delay_latch2, delay_latch1.nget(), PHI1);

		P::Set2(ff2_latch, NOR(nmip_latch.get(), NOR(ff2_latch.get(), delay_latch2.get())), PHI2);

		// Interrupt vector

		P::Set2(zadl_latch[0], NOT(BRK5_RDY), PHI2);
		P::Set2(zadl_latch[1], NOT(NOR(BRK7, NOT(DORES))), PHI2);
		P::Set2(zadl_latch[2], NOR3(BRK7, DORES, n_DONMI), PHI2);

		// Outputs

		core->wire.BRK6E = BRK6E;
		core->wire.BRK7 = BRK7;
		core->wire.DORES = DORES;
		if constexpr (Packed) commands |= (uint64_t(zadl_latch[0].nget()) & 1u) << 37;
		else core->cmd.Z_ADL0 = zadl_latch[0].nget();
		if constexpr (Packed) commands |= (uint64_t(zadl_latch[1].nget()) & 1u) << 38;
		else core->cmd.Z_ADL1 = zadl_latch[1].nget();
		if constexpr (Packed) commands |= (uint64_t(NOT(zadl_latch[2].nget())) & 1u) << 39;
		else core->cmd.Z_ADL2 = NOT(zadl_latch[2].nget());
		core->wire.n_DONMI = n_DONMI;
		core->wire.BRK5_RDY = BRK5_RDY;

		if constexpr (Packed) core->cmd.raw = (core->cmd.raw & ~UINT64_C(962072674304)) | commands;
	}

	void BRKProcessing::sim_AfterRandom()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked) sim_AfterRandomImpl<true>();
		else sim_AfterRandomImpl<false>();
	}

	template<bool Packed, int Phase> void BRKProcessing::sim_AfterRandomImpl()
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState T0 = core->wire.T0;
		TriState BR2 = DecoderView<Packed>(core->decoder_out,core->packed_decoder)[80];
		TriState n_I_OUT = core->random->flags->getn_I_OUT(core->wire.BRK6E);
		TriState n_IRQP = core->wire.n_IRQP;
		TriState n_DONMI = core->wire.n_DONMI;
		TriState BRK6E = core->wire.BRK6E;
		TriState DORES = core->wire.DORES;

		// B Flag

		TriState int_set = NAND(
			OR(BR2, T0),
			NAND(n_DONMI, OR(n_IRQP, NOT(n_I_OUT)))
		);

		P::Set1(b_latch2, NOR(b_latch1.get(), BRK6E), PHI1);
		P::Set2(b_latch1, AND(int_set, NOT(b_latch2.get())), PHI2);
		TriState B_OUT = NOR(DORES, NOR(b_latch1.get(), BRK6E));

		core->wire.B_OUT = B_OUT;
	}

	// PHI1, second traversal, the settling part of sim_BeforeRandom: BRK5_RDY and BRK7 read the
	// re-selected decoder row; donmi_latch reads delay_latch2, which the first traversal rewrote after
	// this read, and n_DONMI follows. brk6_latch1 and res_latch2 are fixed points of their recurrences
	// (brk6_latch1 <- a & (~n_ready | brk6_latch1), res_latch2 <- (res_latch1 | res_latch2) & ~BRK6E, both
	// over PHI2 state), brk6e_latch, nmip_latch and delay_latch2 copy unchanged signals, the PHI2 latches
	// are not enabled, and BRK6E, DORES and the Z_ADL command bits are unchanged.
	void BRKProcessing::sim_SettlePhi1BeforeRandom()
	{
		TriState BRK5 = DecoderView<true>(core->decoder_out, core->packed_decoder)[22];
		TriState n_ready = core->wire.n_ready;
		TriState n_NMIP = core->wire.n_NMIP;

		TriState BRK5_RDY = AND(BRK5, NOT(n_ready));
		TriState BRK7 = NOR(brk6_latch1.nget(), BRK5_RDY);

		donmi_latch.set(NOR3(brk7_latch.nget(), n_NMIP, NOR(nmip_latch.get(), NOR(ff2_latch.get(), delay_latch2.get()))), TriState::One);
		TriState n_DONMI = NOR(donmi_latch.get(), NOR(ff1_latch.get(), brk6e_latch.get()));

		core->wire.BRK7 = BRK7;
		core->wire.n_DONMI = n_DONMI;
		core->wire.BRK5_RDY = BRK5_RDY;
	}

	TriState BRKProcessing::getDORES()
	{
		TriState DORES = NOT(NOR(res_latch1.get(), res_latch2.get()));
		return DORES;
	}

	TriState BRKProcessing::getB_OUT(TriState BRK6E)
	{
		TriState DORES = getDORES();
		TriState B_OUT = NOR(DORES, NOR(b_latch1.get(), BRK6E));
		return B_OUT;
	}

	TriState BRKProcessing::getn_BRK6_LATCH2()
	{
		return brk6_latch2.nget();
	}
	// Out-of-line specializations also serve the typed CPU execution path.
	template void BRKProcessing::sim_BeforeRandomImpl<false, 0>();
	template void BRKProcessing::sim_BeforeRandomImpl<true, 0>();
	template void BRKProcessing::sim_BeforeRandomImpl<true, 1>();
	template void BRKProcessing::sim_BeforeRandomImpl<true, 2>();
	template void BRKProcessing::sim_AfterRandomImpl<false, 0>();
	template void BRKProcessing::sim_AfterRandomImpl<true, 0>();
	template void BRKProcessing::sim_AfterRandomImpl<true, 1>();
	template void BRKProcessing::sim_AfterRandomImpl<true, 2>();

}
