#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	ALUControl::ALUControl(M6502* parent)
	{
		core = parent;

		if (core->GetExecutionBackend() == ExecutionBackend::Reference)
		{
			temp_tab1 = new CarryBCD_TempWire[1 << 19];
			size_t maxVal = 1ULL << 19;
			for (size_t n = 0; n < maxVal; n++)
			{
				uint8_t ir = n & 0xff;
				bool n_T0 = (n >> 8) & 1;
				bool n_T1X = (n >> 9) & 1;
				bool n_T2 = (n >> 10) & 1;
				bool n_T3 = (n >> 11) & 1;
				bool n_T4 = (n >> 12) & 1;
				bool n_T5 = (n >> 13) & 1;
				bool n_ready = (n >> 14) & 1;
				bool T0 = (n >> 15) & 1;
				bool RMW_T6 = (n >> 16) & 1;
				bool BRFW = (n >> 17) & 1;
				bool n_C_OUT = (n >> 18) & 1;

				temp_tab1[n] = PreCalc1(ir, n_T0, n_T1X, n_T2, n_T3, n_T4, n_T5,
					n_ready, T0, RMW_T6, BRFW, n_C_OUT);
			}

		}

		prev_temp1.bits = 0xff;
	}

	void ALUControl::sim()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked) simImpl<true>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
		else simImpl<false>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
	}

	// Bank bits of each packed block (random_logic.h); their union is AluL1Mask / AluL2Mask.
	namespace
	{
		constexpr uint64_t CarryBcdL2 = LatchMask(L2_ACIN1) | LatchMask(L2_ACIN2) | LatchMask(L2_ACIN3) | LatchMask(L2_ACIN4) | LatchMask(L2_DAA1) | LatchMask(L2_DSA1);
		constexpr uint64_t CarryBcdL1 = LatchMask(L1_ACIN5) | LatchMask(L1_DAA2) | LatchMask(L1_DSA2);
		constexpr uint64_t AluInputL2 = LatchMask(L2_NDBADD) | LatchMask(L2_DBADD) | LatchMask(L2_ZADD) | LatchMask(L2_SBADD) | LatchMask(L2_ADLADD);
		constexpr uint64_t AluOpsL2 = LatchMask(L2_ANDS1) | LatchMask(L2_EORS1) | LatchMask(L2_ORS1) | LatchMask(L2_SRS1) | LatchMask(L2_SUMS1);
		constexpr uint64_t AluOpsL1 = LatchMask(L1_ANDS2) | LatchMask(L1_EORS2) | LatchMask(L1_ORS2) | LatchMask(L1_SRS2) | LatchMask(L1_SUMS2);
		constexpr uint64_t AddOutL2 = LatchMask(L2_ADDSB7) | LatchMask(L2_ADDSB06) | LatchMask(L2_ADDADL);
		constexpr uint64_t AddSb7L2 = LatchMask(L2_COUT) | LatchMask(L2_MUX1) | LatchMask(L2_SR1) | LatchMask(L2_FF2);
		constexpr uint64_t AddSb7L1 = LatchMask(L1_ALU_NREADY) | LatchMask(L1_SR2) | LatchMask(L1_FF1);
		static_assert((CarryBcdL2 | AluInputL2 | AluOpsL2 | AddOutL2 | AddSb7L2) == AluL2Mask, "ALUControl PHI2 latch bank");
		static_assert((CarryBcdL1 | AluOpsL1 | AddSb7L1) == AluL1Mask, "ALUControl PHI1 latch bank");
	}

	// Public block entry points (ExactPacked): merge the block's new latch values under the run-time enables.
	void ALUControl::CommitLatches(uint64_t w1, uint64_t w2, uint64_t mask1, uint64_t mask2)
	{
		if (core->wire.PHI1 == TriState::One) core->cmd_latch1 = (core->cmd_latch1 & ~mask1) | (w1 & mask1);
		if (core->wire.PHI2 == TriState::One) core->cmd_latch2 = (core->cmd_latch2 & ~mask2) | (w2 & mask2);
	}

	template<bool Packed, int Phase> void ALUControl::simImpl(uint64_t& command_word, uint64_t& b1, uint64_t& b2)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState n_ready = core->wire.n_ready;
		TriState RMW_T6 = core->wire.RMW_T6;
		TriState n_C_OUT = core->random->flags->getn_C_OUT();

		if constexpr (Packed)
		{
			// Bit-bank form of the latches (random_logic.h). w1/w2 collect this traversal's values of every
			// ALUControl latch enabled by PHI1/PHI2 and are merged into the banks once, before the outputs.
			// No ALUControl latch is read after being written in the same traversal: a PHI2 latch feeds only
			// PHI1 latches (sr1 -> sr2, mux1/cout -> ff1, daa1 -> daa2, dsa1 -> dsa2, acin1..4 -> acin5,
			// the ands..sums chain) and outputs, a PHI1 latch only PHI2 latches (nready -> mux1, ff1 -> ff2),
			// n_ADD_SB7 (consumed in PHI2 only) and outputs, and one phase is high per traversal.
			uint64_t w1 = 0, w2 = 0;

			w1 |= LatchBit(n_ready, L1_ALU_NREADY);

			if (P::Phi2(PHI2)) STKOP = NOR(LatchGet(b1, L1_ALU_NREADY), NOT(d[D_STK_ANY]));	// NOR6(d21..d26) == NOT(STK_ANY)

			sim_CarryBCDImpl<Packed, Phase>(b1, b2, w1, w2);

			sim_ALUInputImpl<Packed, Phase>(b1, b2, w1, w2);

			sim_ALUOpsImpl<Packed, Phase>(b1, b2, w1, w2);

			// ADD/SB7

			TriState SR = NOT(NOR(d[75], AND(d[76], RMW_T6)));
			TriState n_ROR = NOT(d[27]);
			w2 |= LatchBit(NOT(n_C_OUT), L2_COUT);
			w2 |= LatchBit(NOR(LatchGet(b1, L1_ALU_NREADY), NOT(SR)), L2_MUX1);
			w2 |= LatchBit(SR, L2_SR1);
			w1 |= LatchBit(LatchNGet(b2, L2_SR1), L1_SR2);
			w1 |= LatchBit(NOT(MUX(LatchGet(b2, L2_MUX1), LatchGet(b2, L2_FF2), LatchNGet(b2, L2_COUT))), L1_FF1);
			w2 |= LatchBit(LatchNGet(b1, L1_FF1), L2_FF2);
			// n_ADD_SB7 is consumed only by the ADD/SB06-SB7-ADL block of this same traversal.
			const TriState n_ADD_SB7 = NOR3(LatchNGet(b1, L1_FF1), LatchGet(b1, L1_SR2), n_ROR);

			sim_ADDOutImpl<Packed, Phase>(b1, b2, w1, w2, n_ADD_SB7);

			if (P::Phi2One(PHI2)) b2 = (b2 & ~AluL2Mask) | w2;
			if (P::Phi1One(PHI1)) b1 = (b1 & ~AluL1Mask) | w1;

			// Outputs: NDB_ADD..ADL_ADD are NOT(latch) in PHI1 and zero in PHI2; ADD_SB7 is its latch, ADD_SB06 and
			// ADD_ADL their nget(); n_ACIN is acin_latch5 and ANDS..SUMS, n_DAA, n_DSA the nget() of the PHI1 chain.

			constexpr uint64_t operand_loads = AluInputL2;
			constexpr uint64_t chain = LatchMask(L1_ACIN5) | AluOpsL1 | LatchMask(L1_DAA2) | LatchMask(L1_DSA2);
			const uint64_t commands = ((~b2 ^ LatchMask(L2_ADDSB7)) & (AddOutL2 | (P::Phi2One(PHI2) ? 0 : operand_loads))) |
				((~b1 ^ LatchMask(L1_ACIN5)) & chain);
			command_word = (command_word & ~UINT64_C(16776960)) | commands;
		}
		else
		{
			(void)b1; (void)b2;
			P::Set1(nready_latch, n_ready, PHI1);

			if (P::Phi2(PHI2))
			{
				TriState n3[6];
				n3[0] = d[21];
				n3[1] = d[22];
				n3[2] = d[23];
				n3[3] = d[24];
				n3[4] = d[25];
				n3[5] = d[26];
				STKOP = NOR(nready_latch.get(), NOR6(n3));
			}

			uint64_t unused1 = 0, unused2 = 0;
			sim_CarryBCDImpl<Packed, Phase>(b1, b2, unused1, unused2);

			sim_ALUInputImpl<Packed, Phase>(b1, b2, unused1, unused2);

			sim_ALUOpsImpl<Packed, Phase>(b1, b2, unused1, unused2);

			// ADD/SB7

			TriState SR = NOT(NOR(d[75], AND(d[76], RMW_T6)));
			TriState n_ROR = NOT(d[27]);
			P::Set2(cout_latch, NOT(n_C_OUT), PHI2);
			P::Set2(mux_latch1, NOR(nready_latch.get(), NOT(SR)), PHI2);
			P::Set2(sr_latch1, SR, PHI2);
			P::Set1(sr_latch2, sr_latch1.nget(), PHI1);
			P::Set1(ff_latch1, NOT(MUX(mux_latch1.get(), ff_latch2.get(), cout_latch.nget())), PHI1);
			P::Set2(ff_latch2, ff_latch1.nget(), PHI2);
			// n_ADD_SB7 is consumed only by the ADD/SB06-SB7-ADL block of this same traversal.
			const TriState n_ADD_SB7 = NOR3(ff_latch1.nget(), sr_latch2.get(), n_ROR);

			sim_ADDOutImpl<Packed, Phase>(b1, b2, unused1, unused2, n_ADD_SB7);

			// Outputs

			if (P::Phi2One(PHI2))
			{
				core->cmd.NDB_ADD = 0;
				core->cmd.DB_ADD = 0;
				core->cmd.Z_ADD = 0;
				core->cmd.SB_ADD = 0;
				core->cmd.ADL_ADD = 0;
			}
			else
			{
				core->cmd.NDB_ADD = NOT(ndbadd_latch.get());
				core->cmd.DB_ADD = NOT(dbadd_latch.get());
				core->cmd.Z_ADD = NOT(zadd_latch.get());
				core->cmd.SB_ADD = NOT(sbadd_latch.get());
				core->cmd.ADL_ADD = NOT(adladd_latch.get());
			}

			core->cmd.ADD_SB7 = addsb7_latch.get();	// care!
			core->cmd.ADD_SB06 = addsb06_latch.nget();
			core->cmd.ADD_ADL = addadl_latch.nget();

			core->cmd.ANDS = ands_latch2.nget();
			core->cmd.EORS = eors_latch2.nget();
			core->cmd.ORS = ors_latch2.nget();
			core->cmd.SRS = srs_latch2.nget();
			core->cmd.SUMS = sums_latch2.nget();

			core->cmd.n_ACIN = NOT(acin_latch5.nget());
			core->cmd.n_DAA = daa_latch2.nget();
			core->cmd.n_DSA = dsa_latch2.nget();
		}
	}

	void ALUControl::sim_CarryBCD()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked)
		{
			uint64_t w1 = 0, w2 = 0;
			sim_CarryBCDImpl<true>(core->cmd_latch1, core->cmd_latch2, w1, w2);
			CommitLatches(w1, w2, CarryBcdL1, CarryBcdL2);
		}
		else
		{
			uint64_t unused1 = 0, unused2 = 0;
			sim_CarryBCDImpl<false>(core->cmd_latch1, core->cmd_latch2, unused1, unused2);
		}
	}

	template<bool Packed, int Phase> void ALUControl::sim_CarryBCDImpl(uint64_t& b1, uint64_t& b2, uint64_t& w1, uint64_t& w2)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState n_ready = core->wire.n_ready;
		TriState T0 = core->wire.T0;
		TriState RMW_T6 = core->wire.RMW_T6;
		TriState BRFW = core->random->branch_logic->getBRFW();
		TriState n_C_OUT = core->random->flags->getn_C_OUT();
		TriState n_D_OUT = core->random->flags->getn_D_OUT();

		TriState SBC0 = d[51];

		// Additional signals (/ACIN, /DAA, /DSA)

		if constexpr (Packed)
		{
			(void)b1; (void)PHI1;
			if (P::Phi2One(PHI2))
			{
				CarryBCD_TempWire temp{};
				temp.bits = core->packed_decoder->AluControls(n_ready != Zero,T0 != Zero,RMW_T6 != Zero,BRFW != Zero,n_C_OUT != Zero);

				// The four descriptor latches always hold the descriptor last written (see RegsControl).
				prev_temp1.bits = temp.bits;
				w2 |= LatchBit(temp.n_ADL_ADD_Derived ? TriState::One : TriState::Zero, L2_ACIN1);
				w2 |= LatchBit(temp.INC_SB ? TriState::One : TriState::Zero, L2_ACIN2);
				w2 |= LatchBit(temp.BRX ? TriState::One : TriState::Zero, L2_ACIN3);
				w2 |= LatchBit(temp.CSET ? TriState::One : TriState::Zero, L2_ACIN4);

				n_ADL_ADD = temp.n_ADL_ADD ? TriState::One : TriState::Zero;
				INC_SB = temp.INC_SB ? TriState::One : TriState::Zero;
				BRX = temp.BRX ? TriState::One : TriState::Zero;
			}
			else
			{
				TriState n_ACIN = NOR4(LatchGet(b2, L2_ACIN1), LatchGet(b2, L2_ACIN2), LatchGet(b2, L2_ACIN3), LatchGet(b2, L2_ACIN4));
				w1 |= LatchBit(n_ACIN, L1_ACIN5);
			}

			TriState D_OUT = NOT(n_D_OUT);
			w2 |= LatchBit(NOT(NOR(NAND(d[52], D_OUT), SBC0)), L2_DAA1);
			w1 |= LatchBit(LatchNGet(b2, L2_DAA1), L1_DAA2);

			w2 |= LatchBit(NAND(SBC0, D_OUT), L2_DSA1);
			w1 |= LatchBit(LatchNGet(b2, L2_DSA1), L1_DSA2);
		}
		else
		{
			(void)b1; (void)b2; (void)w1; (void)w2;
			if (P::Phi2One(PHI2))
			{
				// Vector: IR+TX, n_ready, T0, RMW_T6, BRFW, n_C_OUT  (lsb -> msb)

				size_t n = 0;
				n |= core->ir->IROut;
				n |= ((size_t)core->TxBits << 8);
				n |= ((size_t)n_ready << 14);
				n |= ((size_t)T0 << 15);
				n |= ((size_t)RMW_T6 << 16);
				n |= ((size_t)BRFW << 17);
				n |= ((size_t)n_C_OUT << 18);

				CarryBCD_TempWire temp = temp_tab1[n];

				if (prev_temp1.bits != temp.bits)
				{
					P::Set2(acin_latch1, temp.n_ADL_ADD_Derived ? TriState::One : TriState::Zero, PHI2);
					P::Set2(acin_latch2, temp.INC_SB ? TriState::One : TriState::Zero, PHI2);
					P::Set2(acin_latch3, temp.BRX ? TriState::One : TriState::Zero, PHI2);
					P::Set2(acin_latch4, temp.CSET ? TriState::One : TriState::Zero, PHI2);
					prev_temp1.bits = temp.bits;
				}

				n_ADL_ADD = prev_temp1.n_ADL_ADD ? TriState::One : TriState::Zero;
				INC_SB = prev_temp1.INC_SB ? TriState::One : TriState::Zero;
				BRX = prev_temp1.BRX ? TriState::One : TriState::Zero;
			}
			else
			{
				TriState acin[4]{};
				acin[0] = acin_latch1.get();
				acin[1] = acin_latch2.get();
				acin[2] = acin_latch3.get();
				acin[3] = acin_latch4.get();
				TriState n_ACIN = NOR4(acin);
				P::Set1(acin_latch5, n_ACIN, PHI1);
			}

			TriState D_OUT = NOT(n_D_OUT);
			P::Set2(daa_latch1, NOT(NOR(NAND(d[52], D_OUT), SBC0)), PHI2);
			P::Set1(daa_latch2, daa_latch1.nget(), PHI1);

			P::Set2(dsa_latch1, NAND(SBC0, D_OUT), PHI2);
			P::Set1(dsa_latch2, dsa_latch1.nget(), PHI1);
		}
	}

	void ALUControl::sim_ALUInput()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked)
		{
			uint64_t w1 = 0, w2 = 0;
			sim_ALUInputImpl<true>(core->cmd_latch1, core->cmd_latch2, w1, w2);
			CommitLatches(w1, w2, 0, AluInputL2);
		}
		else
		{
			uint64_t unused1 = 0, unused2 = 0;
			sim_ALUInputImpl<false>(core->cmd_latch1, core->cmd_latch2, unused1, unused2);
		}
	}

	template<bool Packed, int Phase> void ALUControl::sim_ALUInputImpl(uint64_t& b1, uint64_t& b2, uint64_t& w1, uint64_t& w2)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		(void)b1; (void)b2; (void)w1;
		if constexpr (Packed) { if (!P::Phi2One(core->wire.PHI2)) return; }
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI2 = core->wire.PHI2;
		TriState n_ready = core->wire.n_ready;
		TriState BRK6E = core->wire.BRK6E;

		TriState SBC0 = d[51];
		TriState JSR_5 = d[56];

		// Setting the ALU input values (NDB/ADD, DB/ADD, 0/ADD, SB/ADD, ADL/ADD)

		if (P::Phi2One(PHI2))
		{
			TriState n_NDB_ADD = NAND(OR3(BRX, JSR_5, SBC0), NOT(n_ready));
			if constexpr (Packed)
			{
				w2 |= LatchBit(n_NDB_ADD, L2_NDBADD);
				w2 |= LatchBit(NAND(n_NDB_ADD, n_ADL_ADD), L2_DBADD);
				w2 |= LatchBit(n_ADL_ADD, L2_ADLADD);

				// NOR9(STKOP, d30, d31, d45, d48, INC_SB, d47, BRK6E, n_ready) with the five lines folded
				TriState SB_ADD = NOR5(STKOP, d[D_SBADD_ANY], INC_SB, BRK6E, n_ready);
				w2 |= LatchBit(NOT(SB_ADD), L2_SBADD);
				w2 |= LatchBit(SB_ADD, L2_ZADD);
			}
			else
			{
				(void)w2;
				P::Set2(ndbadd_latch, n_NDB_ADD, PHI2);
				P::Set2(dbadd_latch, NAND(n_NDB_ADD, n_ADL_ADD), PHI2);
				P::Set2(adladd_latch, n_ADL_ADD, PHI2);

				TriState JSR2 = d[48];
				TriState RET = d[47];
				TriState sbadd[9]{};
				sbadd[0] = STKOP;
				sbadd[1] = d[30];
				sbadd[2] = d[31];
				sbadd[3] = d[45];
				sbadd[4] = JSR2;
				sbadd[5] = INC_SB;
				sbadd[6] = RET;
				sbadd[7] = BRK6E;
				sbadd[8] = n_ready;
				TriState SB_ADD = NOR9(sbadd);
				P::Set2(sbadd_latch, NOT(SB_ADD), PHI2);
				P::Set2(zadd_latch, SB_ADD, PHI2);
			}
		}
	}

	void ALUControl::sim_ALUOps()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked)
		{
			uint64_t w1 = 0, w2 = 0;
			sim_ALUOpsImpl<true>(core->cmd_latch1, core->cmd_latch2, w1, w2);
			CommitLatches(w1, w2, AluOpsL1, AluOpsL2);
		}
		else
		{
			uint64_t unused1 = 0, unused2 = 0;
			sim_ALUOpsImpl<false>(core->cmd_latch1, core->cmd_latch2, unused1, unused2);
		}
	}

	template<bool Packed, int Phase> void ALUControl::sim_ALUOpsImpl(uint64_t& b1, uint64_t& b2, uint64_t& w1, uint64_t& w2)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		(void)b1;
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState n_ready = core->wire.n_ready;
		TriState RMW_T6 = core->wire.RMW_T6;

		TriState EOR = d[29];
		TriState _OR = NOT(NOR(d[32], n_ready));
		TriState _AND = Packed ? d[D_AND] : NOT(NOR(d[69], d[70]));
		TriState SR = NOT(NOR(d[75], AND(d[76], RMW_T6)));

		// ALU operation commands (ANDS, EORS, ORS, SRS, SUMS)

		if (P::Phi2One(PHI2))
		{
			TriState sums[4];
			sums[0] = _AND;
			sums[1] = EOR;
			sums[2] = _OR;
			sums[3] = SR;
			if constexpr (Packed)
			{
				(void)w1;
				w2 |= LatchBit(_AND, L2_ANDS1);
				w2 |= LatchBit(EOR, L2_EORS1);
				w2 |= LatchBit(_OR, L2_ORS1);
				w2 |= LatchBit(SR, L2_SRS1);
				w2 |= LatchBit(NOR4(sums), L2_SUMS1);
			}
			else
			{
				(void)b2; (void)w1; (void)w2;
				P::Set2(ands_latch1, _AND, PHI2);
				P::Set2(eors_latch1, EOR, PHI2);
				P::Set2(ors_latch1, _OR, PHI2);
				P::Set2(srs_latch1, SR, PHI2);

				P::Set2(sums_latch1, NOR4(sums), PHI2);
			}
		}
		else
		{
			if constexpr (Packed)
			{
				// ands..sums latch2 = latch1.nget(): the PHI1 chain copies sit at the PHI2 sources' bit positions.
				(void)PHI1; (void)w2;
				w1 |= ~b2 & AluOpsL2;
			}
			else
			{
				(void)b2; (void)w1; (void)w2;
				P::Set1(ands_latch2, ands_latch1.nget(), PHI1);
				P::Set1(eors_latch2, eors_latch1.nget(), PHI1);
				P::Set1(ors_latch2, ors_latch1.nget(), PHI1);
				P::Set1(srs_latch2, srs_latch1.nget(), PHI1);

				P::Set1(sums_latch2, sums_latch1.nget(), PHI1);
			}
		}
	}

	void ALUControl::sim_ADDOut(TriState n_ADD_SB7)
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked)
		{
			uint64_t w1 = 0, w2 = 0;
			sim_ADDOutImpl<true>(core->cmd_latch1, core->cmd_latch2, w1, w2, n_ADD_SB7);
			CommitLatches(w1, w2, 0, AddOutL2);
		}
		else
		{
			uint64_t unused1 = 0, unused2 = 0;
			sim_ADDOutImpl<false>(core->cmd_latch1, core->cmd_latch2, unused1, unused2, n_ADD_SB7);
		}
	}

	template<bool Packed, int Phase> void ALUControl::sim_ADDOutImpl(uint64_t& b1, uint64_t& b2, uint64_t& w1, uint64_t& w2, TriState n_ADD_SB7)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		(void)b1; (void)b2; (void)w1;
		if constexpr (Packed) { if (!P::Phi2One(core->wire.PHI2)) return; }
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI2 = core->wire.PHI2;
		TriState T1 = core->disp->getT1();
		TriState RMW_T7 = core->wire.RMW_T7;

		TriState JSR_5 = d[56];

		TriState BR0 = AND(d[73], NOT(core->wire.n_PRDY));
		// NAND(NOR(d71, d72), NOT(BR0)) == OR(NOT(NOR(d71, d72)), BR0)
		TriState PGX = Packed ? OR(d[D_PGX_STATIC], BR0) : NAND(NOR(d[71], d[72]), NOT(BR0));

		// Control commands of the intermediate ALU result (ADD/SB06, ADD/SB7, ADD/ADL)

		if (P::Phi2One(PHI2))
		{
			TriState sb06[5]{};
			sb06[0] = RMW_T7;
			sb06[1] = STKOP;
			sb06[2] = JSR_5;
			sb06[3] = T1;
			sb06[4] = PGX;
			TriState n_ADD_SB06 = NOR5(sb06);

			TriState NOADL;
			if constexpr (Packed) NOADL = d[D_NOADL];
			else
			{
				TriState RTS_5 = d[84];
				TriState RTI_5 = d[26];
				TriState noadl[7]{};
				noadl[0] = RTS_5;
				noadl[1] = d[85];
				noadl[2] = d[86];
				noadl[3] = d[87];
				noadl[4] = d[88];
				noadl[5] = d[89];
				noadl[6] = RTI_5;
				NOADL = NOR7(noadl);
			}

			if constexpr (Packed)
			{
				w2 |= LatchBit(n_ADD_SB06, L2_ADDSB06);
				w2 |= LatchBit(NOR(n_ADD_SB06, n_ADD_SB7), L2_ADDSB7);
				w2 |= LatchBit(NOT(NOR(NOADL, PGX)), L2_ADDADL);
			}
			else
			{
				(void)w2;
				P::Set2(addsb06_latch, n_ADD_SB06, PHI2);

				P::Set2(addsb7_latch, NOR(n_ADD_SB06, n_ADD_SB7), PHI2);

				P::Set2(addadl_latch, NOT(NOR(NOADL, PGX)), PHI2);
			}
		}
	}

	CarryBCD_TempWire ALUControl::PreCalc1(uint8_t ir, bool n_T0, bool n_T1X, bool n_T2, bool n_T3, bool n_T4, bool n_T5,
		bool n_ready, bool T0, bool RMW_T6, bool BRFW, bool n_C_OUT)
	{
		TriState* d;
		DecoderInput decoder_in{};
		decoder_in.packed_bits = 0;
		CarryBCD_TempWire temp{};
		temp.bits = 0;

		TriState IR0 = ir & 0b00000001 ? TriState::One : TriState::Zero;
		TriState IR1 = ir & 0b00000010 ? TriState::One : TriState::Zero;
		TriState IR2 = ir & 0b00000100 ? TriState::One : TriState::Zero;
		TriState IR3 = ir & 0b00001000 ? TriState::One : TriState::Zero;
		TriState IR4 = ir & 0b00010000 ? TriState::One : TriState::Zero;
		TriState IR5 = ir & 0b00100000 ? TriState::One : TriState::Zero;
		TriState IR6 = ir & 0b01000000 ? TriState::One : TriState::Zero;
		TriState IR7 = ir & 0b10000000 ? TriState::One : TriState::Zero;

		decoder_in.n_IR0 = NOT(IR0);
		decoder_in.n_IR1 = NOT(IR1);
		decoder_in.IR01 = OR(IR0, IR1);
		decoder_in.n_IR2 = NOT(IR2);
		decoder_in.IR2 = IR2;
		decoder_in.n_IR3 = NOT(IR3);
		decoder_in.IR3 = IR3;
		decoder_in.n_IR4 = NOT(IR4);
		decoder_in.IR4 = IR4;
		decoder_in.n_IR5 = NOT(IR5);
		decoder_in.IR5 = IR5;
		decoder_in.n_IR6 = NOT(IR6);
		decoder_in.IR6 = IR6;
		decoder_in.n_IR7 = NOT(IR7);
		decoder_in.IR7 = IR7;

		decoder_in.n_T0 = n_T0;
		decoder_in.n_T1X = n_T1X;
		decoder_in.n_T2 = n_T2;
		decoder_in.n_T3 = n_T3;
		decoder_in.n_T4 = n_T4;
		decoder_in.n_T5 = n_T5;

		core->decoder->sim(decoder_in.packed_bits, &d);

		// Wires

		TriState RET = d[47];
		TriState BR3 = d[93];

		TriState adladd[7]{};
		adladd[0] = AND(d[33], NOT(d[34]));
		adladd[1] = d[35];	// STK2
		adladd[2] = d[36];
		adladd[3] = d[37];
		adladd[4] = d[38];
		adladd[5] = d[39];
		adladd[6] = n_ready ? TriState::One : TriState::Zero;
		temp.n_ADL_ADD = NOR7(adladd);

		TriState incsb[6]{};
		incsb[0] = d[39];
		incsb[1] = d[40];
		incsb[2] = d[41];
		incsb[3] = d[42];
		incsb[4] = d[43];
		incsb[5] = AND(RMW_T6 ? TriState::One : TriState::Zero, d[44]);
		temp.INC_SB = NOT(NOR6(incsb));

		temp.BRX = NOT(NOR3(d[49], d[50], NOR(NOT(BR3), BRFW ? TriState::One : TriState::Zero)));

		temp.CSET = NAND(NAND(NOR(n_C_OUT ? TriState::One : TriState::Zero, NOR(T0 ? TriState::One : TriState::Zero, RMW_T6 ? TriState::One : TriState::Zero)), OR(d[52], d[53])), NOT(d[54]));

		temp.n_ADL_ADD_Derived = NOR(temp.n_ADL_ADD ? TriState::One : TriState::Zero, NOT(RET));

		return temp;
	}

	// Out-of-line specializations also serve the typed CPU execution path.
	template void ALUControl::simImpl<false, 0>(uint64_t&, uint64_t&, uint64_t&);
	template void ALUControl::simImpl<true, 0>(uint64_t&, uint64_t&, uint64_t&);
	template void ALUControl::simImpl<true, 1>(uint64_t&, uint64_t&, uint64_t&);
	template void ALUControl::simImpl<true, 2>(uint64_t&, uint64_t&, uint64_t&);

}
