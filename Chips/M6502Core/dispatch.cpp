#include "pch.h"

using namespace BaseLogic;

// The dispatcher simulation is divided into three stages: before decoder, before random logic, and after random logic.

// So far in this form (critical mass of code). In the process of debugging, it is possible to rearrange some sections.

namespace M6502Core
{
	void Dispatcher::sim_BeforeDecoder()
	{
		sim_BeforeDecoderImpl<0>();
	}

	template<int Phase> void Dispatcher::sim_BeforeDecoderImpl()
	{
		using P = Phi<Phase>;
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState RDY = core->wire.RDY;
		TriState DORES = core->brk->getDORES();
		TriState ACR = core->alu->getACR();

		// Processor Readiness

		TriState WR = NOR3(NOT(ready_latch1.nget()), wr_latch.get(), DORES);
		P::Set1(ready_latch2, WR, PHI1);
		P::Set2(ready_latch1, NOR(RDY, ready_latch2.get()), PHI2);
		TriState n_ready = NOT(ready_latch1.nget());

		// Ready Delay

		P::Set1(rdydelay_latch1, n_ready, PHI1);
		P::Set2(rdydelay_latch2, rdydelay_latch1.nget(), PHI2);
		TriState NotReadyPhi1 = rdydelay_latch2.nget();

		// ACRL

		TriState ACRL1 = NOR(AND(NOT(ACR), NOT(NotReadyPhi1)), NOR(NOT(NotReadyPhi1), acr_latch2.nget()));
		P::Set1(acr_latch1, ACRL1, PHI1);
		P::Set2(acr_latch2, acr_latch1.nget(), PHI2);
		TriState ACRL2 = acr_latch2.nget();

		// Short Cycle Counter

		P::Set1(t1x_latch, NOR(t0_latch.get(), n_ready), PHI1);
		TriState n_T0 = NOR(NOR(comp_latch1.get(), AND(comp_latch2.get(), comp_latch3.get())), NOR(t0_latch.get(), t1x_latch.get()));
		P::Set2(t0_latch, n_T0, PHI2);
		TriState T0 = NOT(n_T0);
		TriState n_T1X = t1x_latch.nget();

		// Opcode Fetch

		TriState BRK6E = NOR(core->brk->getn_BRK6_LATCH2(), n_ready);
		TriState B_OUT = core->brk->getB_OUT(BRK6E);

		TriState T1 = t1_latch.nget();
		P::Set2(fetch_latch, T1, PHI2);
		TriState FETCH = NOR(fetch_latch.nget(), n_ready);
		TriState Z_IR = NAND(B_OUT, FETCH);

		// Outputs

		core->wire.T0 = T0;
		core->wire.n_T0 = n_T0;
		core->wire.n_T1X = n_T1X;
		core->wire.Z_IR = Z_IR;
		core->wire.FETCH = FETCH;
		core->wire.n_ready = n_ready;
		core->wire.ACRL1 = ACRL1;
		core->wire.ACRL2 = ACRL2;
	}

	void Dispatcher::sim_BeforeRandomLogic()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked) sim_BeforeRandomLogicImpl<true>();
		else sim_BeforeRandomLogicImpl<false>();
	}

	template<bool Packed, int Phase> void Dispatcher::sim_BeforeRandomLogicImpl()
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState n_ready = core->wire.n_ready;

		TriState n_SHIFT, n_MemOp;
		if constexpr (Packed)
		{
			n_SHIFT = d[D_nSHIFT];
			n_MemOp = d[D_nMemOp];
		}
		else
		{
			n_SHIFT = NOR(d[106], d[107]);

			TriState memop_in[5]{};
			memop_in[0] = d[111];
			memop_in[1] = d[122];
			memop_in[2] = d[123];
			memop_in[3] = d[124];
			memop_in[4] = d[125];
			n_MemOp = NOR5(memop_in);
		}

		// T6 / T7 (RMW)

		P::Set1(t6_latch1, NOR(AND(t6_latch2.get(), n_ready), t67_latch.get()), PHI1);
		P::Set1(t7_latch2, t7_latch1.nget(), PHI1);
		P::Set2(t67_latch, NOR3(n_SHIFT, n_MemOp, n_ready), PHI2);
		P::Set2(t6_latch2, t6_latch1.nget(), PHI2);
		TriState RMW_T6 = t6_latch1.nget();
		P::Set2(t7_latch1, NAND(RMW_T6, NOT(n_ready)), PHI2);
		TriState RMW_T7 = NOT(t7_latch2.nget());

		core->wire.RMW_T6 = RMW_T6;
		core->wire.RMW_T7 = RMW_T7;
	}

	void Dispatcher::sim_AfterRandomLogic()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked) sim_AfterRandomLogicImpl<true>();
		else sim_AfterRandomLogicImpl<false>();
	}

	template<bool Packed, int Phase> void Dispatcher::sim_AfterRandomLogicImpl()
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState BRK6E = core->wire.BRK6E;
		TriState RESP = core->wire.RESP;
		TriState ACR = core->alu->getACR();
		TriState BRFW = core->wire.BRFW;
		TriState n_BRTAKEN = core->wire.n_BRTAKEN;
		TriState n_TWOCYCLE = core->wire.n_TWOCYCLE;
		TriState n_IMPLIED = core->wire.n_IMPLIED;
		TriState PC_DB = core->wire.PC_DB;
		TriState n_ADL_PCL = core->wire.n_ADL_PCL;
		TriState n_ready = core->wire.n_ready;
		TriState T0 = core->wire.T0;
		TriState B_OUT = core->brk->getB_OUT(BRK6E);
		TriState RMW_T6 = core->wire.RMW_T6;
		TriState RMW_T7 = core->wire.RMW_T7;
		TriState ACRL1 = core->wire.ACRL1;
		TriState ACRL2 = core->wire.ACRL2;
		TriState RDY = core->wire.RDY;
		TriState DORES = core->wire.DORES;

		TriState BR2 = d[80];
		TriState BR3 = d[93];

		TriState n_SHIFT = TriState::Zero, n_STORE = TriState::Zero, REST;
		if constexpr (Packed) REST = d[D_REST];
		else
		{
			n_SHIFT = NOR(d[106], d[107]);
			n_STORE = NOT(d[97]);
			REST = NAND(n_SHIFT, n_STORE);
		}

		// Ready Delay (get)

		TriState NotReadyPhi1 = rdydelay_latch2.nget();

		// Increment PC

		P::Set2(br_latch1, NOR(AND(n_BRTAKEN, BR2), NOR(n_ADL_PCL, NOT(NOR(BR2, BR3)))), PHI2);
		P::Set2(br_latch2, NOR(NOT(BR3), NotReadyPhi1), PHI2);
		TriState ipc_temp = AND(XOR(BRFW, NOT(ACR)), NOT(br_latch2.nget()));
		P::Set1(ipc_latch1, B_OUT, PHI1);
		P::Set1(ipc_latch2, ipc_temp, PHI1);
		P::Set1(ipc_latch3, NOR3(n_ready, br_latch1.get(), NOT(n_IMPLIED)), PHI1);
		TriState n_1PC = NAND(ipc_latch1.get(), OR(ipc_latch2.get(), ipc_latch3.get()));

		// Step T1

		P::Set1(nready_latch, NOT(n_ready), PHI1);
		P::Set1(step_latch2, NOR(step_latch1.get(), ipc_temp), PHI1);
		P::Set2(step_latch1, NOR3(nready_latch.get(), RESP, step_latch2.get()), PHI2);

		// Instruction Completion

		TriState ENDS = NOR(ends_latch1.get(), ends_latch2.get());
		TriState n_TRES1 = NOR(NOR(NOR(step_latch1.get(), ipc_temp), n_ready), ENDS);
		P::Set1(t1_latch, n_TRES1, PHI1);
		TriState T1 = t1_latch.nget();
		P::Set2(ends_latch1, MUX(n_ready, NOR(T0, AND(n_BRTAKEN, BR2)), NOT(T1)), PHI2);
		P::Set2(ends_latch2, RESP, PHI2);
		TriState TRES1 = NOT(n_TRES1);

		if constexpr (Packed) P::Set2(tresx_latch1, d[D_TRESX_IN], PHI2);
		else P::Set2(tresx_latch1, NOR(d[91], d[92]), PHI2);

		TriState ENDX;
		TriState n_MemOp = TriState::Zero;

		if (P::Phi2(PHI2))
		{
			if constexpr (Packed)
			{
				// NOR4(static, RMW_T7, static, static) == NOR(NOT(NOR3 of the statics), RMW_T7)
				ENDX = NOR(NOT(d[D_ENDX_STATIC]), RMW_T7);
			}
			else
			{
				TriState memop_in[5]{};
				memop_in[0] = d[111];
				memop_in[1] = d[122];
				memop_in[2] = d[123];
				memop_in[3] = d[124];
				memop_in[4] = d[125];
				n_MemOp = NOR5(memop_in);

				TriState endx_1[6]{};
				endx_1[0] = d[100];
				endx_1[1] = d[101];
				endx_1[2] = d[102];
				endx_1[3] = d[103];
				endx_1[4] = d[104];
				endx_1[5] = d[105];

				TriState endx_2[4]{};
				endx_2[0] = NOR3(d[96], NOT(n_SHIFT), n_MemOp);
				endx_2[1] = RMW_T7;
				endx_2[2] = NOT(NOR6(endx_1));
				endx_2[3] = BR3;

				ENDX = NOR4(endx_2);
			}

			P::Set2(tresx_latch2, NOR3(RESP, ENDS, NOR(n_ready, ENDX)), PHI2);
		}
		else
		{
			ENDX = core->wire.ENDX;
		}

		TriState tresx_nor[4]{};
		tresx_nor[0] = ACRL1;
		tresx_nor[1] = tresx_latch1.get();
		tresx_nor[2] = n_ready;
		tresx_nor[3] = REST;
		TriState n_TRESX = NOR3(NOR4(tresx_nor), BRK6E, tresx_latch2.nget());
		P::Set1(tres2_latch, n_TRESX, PHI1);

		P::Set1(comp_latch1, TRES1, PHI1);
		P::Set1(comp_latch2, n_TWOCYCLE, PHI1);
		P::Set1(comp_latch3, n_TRESX, PHI1);

		// WR

		if (P::Phi2(PHI2))
		{
			if constexpr (Packed)
			{
				// NOR6(STOR, PC_DB, d98, d100, RMW_T6, RMW_T7) == NOR4(NOT(NOR3(STOR, d98, d100)), PC_DB, RMW_T6, RMW_T7)
				P::Set2(wr_latch, NOR4(NOT(d[D_WR_STATIC]), PC_DB, RMW_T6, RMW_T7), PHI2);
			}
			else
			{
				TriState STOR = NOR(n_MemOp, n_STORE);

				TriState wr_in[6]{};
				wr_in[0] = STOR;
				wr_in[1] = PC_DB;
				wr_in[2] = d[98];
				wr_in[3] = d[100];
				wr_in[4] = RMW_T6;
				wr_in[5] = RMW_T7;
				P::Set2(wr_latch, NOR6(wr_in), PHI2);
			}
		}

		// Processor Readiness

		TriState WR = NOR3(NOT(ready_latch1.nget()), wr_latch.get(), DORES);
		P::Set1(ready_latch2, WR, PHI1);
		P::Set2(ready_latch1, NOR(RDY, ready_latch2.get()), PHI2);

		// Outputs

		core->wire.n_1PC = n_1PC;
		core->wire.WR = WR;

		core->wire.ENDS = ENDS;
		core->wire.ENDX = ENDX;
		core->wire.TRES1 = TRES1;
		core->wire.n_TRESX = n_TRESX;
	}

	// PHI2, second traversal, the settling part of sim_BeforeDecoder: ACRL1 reads ACR, which the Bottom
	// traversal in between recomputed, and acr_latch2, which the first traversal rewrote after this
	// block had read it; Z_IR reads brk6_latch2 through BRK6E/B_OUT, likewise rewritten after the read.
	// FETCH and n_ready are the wires the first traversal left (their latches were rewritten with the
	// values they already held).
	void Dispatcher::sim_SettlePhi2BeforeDecoder()
	{
		TriState ACR = core->alu->getACR();
		TriState n_ready = core->wire.n_ready;
		TriState NotReadyPhi1 = rdydelay_latch2.nget();

		TriState ACRL1 = NOR(AND(NOT(ACR), NOT(NotReadyPhi1)), NOR(NOT(NotReadyPhi1), acr_latch2.nget()));

		TriState BRK6E = NOR(core->brk->getn_BRK6_LATCH2(), n_ready);
		TriState B_OUT = core->brk->getB_OUT(BRK6E);
		TriState Z_IR = NAND(B_OUT, core->wire.FETCH);

		core->wire.ACRL1 = ACRL1;
		core->wire.Z_IR = Z_IR;
	}

	// PHI2, second traversal, the settling part of sim_AfterRandomLogic: ENDS reads the ends latches
	// the first traversal rewrote after reading them, ipc_temp reads ACR; n_TRES1/TRES1, tresx_latch2
	// (the one latch of the second traversal whose value can differ) and n_TRESX follow from them and
	// from the settled ACRL1. br_latch2, step_latch1 and tresx_latch1 hold the values the first
	// traversal wrote, which this traversal would rewrite unchanged.
	void Dispatcher::sim_SettlePhi2AfterRandomLogic()
	{
		using P = Phi<2>;
		TriState PHI2 = core->wire.PHI2;
		TriState ACR = core->alu->getACR();
		TriState BRFW = core->wire.BRFW;
		TriState n_ready = core->wire.n_ready;
		TriState RESP = core->wire.RESP;
		TriState BRK6E = core->wire.BRK6E;
		TriState ACRL1 = core->wire.ACRL1;
		TriState ENDX = core->wire.ENDX;
		TriState REST = DecoderView<true>(core->decoder_out, core->packed_decoder)[D_REST];

		TriState ipc_temp = AND(XOR(BRFW, NOT(ACR)), NOT(br_latch2.nget()));

		TriState ENDS = NOR(ends_latch1.get(), ends_latch2.get());
		TriState n_TRES1 = NOR(NOR(NOR(step_latch1.get(), ipc_temp), n_ready), ENDS);
		TriState TRES1 = NOT(n_TRES1);

		P::Set2(tresx_latch2, NOR3(RESP, ENDS, NOR(n_ready, ENDX)), PHI2);

		TriState tresx_nor[4]{};
		tresx_nor[0] = ACRL1;
		tresx_nor[1] = tresx_latch1.get();
		tresx_nor[2] = n_ready;
		tresx_nor[3] = REST;
		TriState n_TRESX = NOR3(NOR4(tresx_nor), BRK6E, tresx_latch2.nget());

		core->wire.ENDS = ENDS;
		core->wire.TRES1 = TRES1;
		core->wire.n_TRESX = n_TRESX;
	}

	// PHI1, second traversal, the settling part of sim_BeforeDecoder: n_T0 reads comp_latch1..3, which
	// the first traversal's sim_AfterRandomLogic rewrote after this block had read them, and Z_IR
	// reads DORES through B_OUT, settled by BRKProcessing after this block's read. Everything else
	// the block writes (ready_latch2, rdydelay_latch1, acr_latch1, t1x_latch) is recomputed from PHI2
	// state or from the settled DORES and equals what the first traversal left; n_ready, ACRL1/2, FETCH
	// and n_T1X are unchanged wires.
	void Dispatcher::sim_SettlePhi1BeforeDecoder()
	{
		TriState n_ready = core->wire.n_ready;

		TriState n_T0 = NOR(NOR(comp_latch1.get(), AND(comp_latch2.get(), comp_latch3.get())), NOR(t0_latch.get(), t1x_latch.get()));
		TriState T0 = NOT(n_T0);

		TriState BRK6E = NOR(core->brk->getn_BRK6_LATCH2(), n_ready);
		TriState B_OUT = core->brk->getB_OUT(BRK6E);
		TriState Z_IR = NAND(B_OUT, core->wire.FETCH);

		core->wire.T0 = T0;
		core->wire.n_T0 = n_T0;
		core->wire.Z_IR = Z_IR;
	}

	TriState Dispatcher::getTRES2()
	{
		return tres2_latch.nget();
	}

	TriState Dispatcher::getT1()
	{
		return t1_latch.nget();
	}

	TriState Dispatcher::getSTOR(TriState d[])
	{
		TriState memop_in[5];
		memop_in[0] = d[111];
		memop_in[1] = d[122];
		memop_in[2] = d[123];
		memop_in[3] = d[124];
		memop_in[4] = d[125];
		TriState n_MemOp = NOR5(memop_in);

		TriState n_STORE = NOT(d[97]);
		TriState STOR = NOR(n_MemOp, n_STORE);

		return STOR;
	}
	// Out-of-line specializations also serve the typed CPU execution path.
	template void Dispatcher::sim_BeforeDecoderImpl<0>();
	template void Dispatcher::sim_BeforeDecoderImpl<1>();
	template void Dispatcher::sim_BeforeDecoderImpl<2>();
	template void Dispatcher::sim_BeforeRandomLogicImpl<false, 0>();
	template void Dispatcher::sim_BeforeRandomLogicImpl<true, 0>();
	template void Dispatcher::sim_BeforeRandomLogicImpl<true, 1>();
	template void Dispatcher::sim_BeforeRandomLogicImpl<true, 2>();
	template void Dispatcher::sim_AfterRandomLogicImpl<false, 0>();
	template void Dispatcher::sim_AfterRandomLogicImpl<true, 0>();
	template void Dispatcher::sim_AfterRandomLogicImpl<true, 1>();
	template void Dispatcher::sim_AfterRandomLogicImpl<true, 2>();

}
