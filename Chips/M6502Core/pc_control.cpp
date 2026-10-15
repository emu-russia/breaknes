#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	void PC_Control::sim()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked) simImpl<true>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
		else simImpl<false>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
	}

	template<bool Packed, int Phase> void PC_Control::simImpl(uint64_t& command_word, uint64_t& b1, uint64_t& b2)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState n_ready = core->wire.n_ready;

		// These two wires are used in the dispatcher, but are only needed during PHI2.

		TriState PC_DB = TriState::Zero;
		TriState n_ADL_PCL = TriState::Zero;

		if constexpr (Packed)
		{
			// Bit-bank form of the latches (random_logic.h): the eight PHI2 output latches are cmd_latch2 bits
			// 27..34 (the command bits they drive) and pch_db_latch1 is bit L2_PCH_DB1; the two PHI1 latches
			// are cmd_latch1 bits L1_PC_NREADY and L1_PCL_DB1. The PHI2 block reads only PHI1 latches and the
			// PHI1 block only PHI2 latches, so each reads stored values.
			if (P::Phi2One(PHI2))
			{
				TriState T0 = core->wire.T0;
				TriState T1 = core->disp->getT1();
				TriState BR0 = AND(d[73], NOT(core->wire.n_PRDY));

				TriState JSR_5 = d[56];
				TriState RTS_5 = d[84];
				TriState BR2 = d[80];
				TriState BR3 = d[93];

				TriState ABS_2 = d[D_ABS_2];
				TriState JB = d[D_JB];
				TriState n_PCH_DB = d[D_nPCH_DB];
				TriState nready_latch_out = LatchGet(b1, L1_PC_NREADY);

				// DB

				TriState n_PCL_DB = LatchNGet(b1, L1_PCL_DB1);
				PC_DB = NAND(n_PCL_DB, n_PCH_DB);

				// ADL

				TriState n1[5]{};
				n1[0] = T1;
				n1[1] = JSR_5;
				n1[2] = ABS_2;
				n1[3] = NOR(NOR(JB, nready_latch_out), NOT(T0));
				n1[4] = BR2;
				TriState n_PCL_ADL = NOR5(n1);

				TriState n2[4]{};
				n2[0] = NOT(n_PCL_ADL);
				n2[1] = RTS_5;
				n2[2] = T0;
				n2[3] = AND(NOT(nready_latch_out), BR3);
				n_ADL_PCL = NOR4(n2);

				// ADH

				TriState DL_PCH = NOR(NOT(T0), JB);
				TriState n_PCH_ADH = NOR(NOR3(n_PCL_ADL, DL_PCH, BR0), BR3);

				// NOR6(RTS_5, ABS_2, T0, T1, BR2, BR3) == NOR3(NOT(NOR4(RTS_5, ABS_2, BR2, BR3)), T0, T1)
				TriState n_ADH_PCH = NOR3(NOT(d[D_NAP_STATIC]), T0, T1);

				const uint64_t w2 = LatchBit(n_PCH_DB, L2_PCH_DB1) | LatchBit(n_PCL_DB, L2_PCL_DB2) | LatchBit(n_PCH_DB, L2_PCH_DB2) |
					LatchBit(n_PCL_ADL, L2_PCL_ADL) | LatchBit(NOT(n_ADL_PCL), L2_PCL_PCL) | LatchBit(n_ADL_PCL, L2_ADL_PCL) |
					LatchBit(n_PCH_ADH, L2_PCH_ADH) | LatchBit(n_ADH_PCH, L2_ADH_PCH) | LatchBit(NOT(n_ADH_PCH), L2_PCH_PCH);
				b2 = (b2 & ~PcL2Mask) | w2;
			}
			else
			{
				// All you have to do during PHI1 is to update these 2 latches.

				const uint64_t w1 = LatchBit(n_ready, L1_PC_NREADY) | LatchBit(NOR(LatchGet(b2, L2_PCH_DB1), n_ready), L1_PCL_DB1);
				if (P::Phi1One(PHI1)) b1 = (b1 & ~PcL1Mask) | w1;
			}

			// Outputs: PCL_DB, PCH_DB, PCL_ADL and PCH_ADH are the latch's nget(); PCL_PCL, ADL_PCL, ADH_PCH and
			// PCH_PCH are NOT(latch) in PHI1 and zero in PHI2.

			constexpr uint64_t stores = LatchMask(L2_PCL_DB2) | LatchMask(L2_PCH_DB2) | LatchMask(L2_PCL_ADL) | LatchMask(L2_PCH_ADH);
			constexpr uint64_t loads = LatchMask(L2_PCL_PCL) | LatchMask(L2_ADL_PCL) | LatchMask(L2_ADH_PCH) | LatchMask(L2_PCH_PCH);
			static_assert((stores | loads) == UINT64_C(34225520640), "PC_Control command bits 27..34");
			const uint64_t commands = ~b2 & (stores | (P::Phi2One(PHI2) ? 0 : loads));

			// For dispatcher

			core->wire.PC_DB = PC_DB;
			core->wire.n_ADL_PCL = n_ADL_PCL;

			command_word = (command_word & ~UINT64_C(34225520640)) | commands;
		}
		else
		{
			(void)b1; (void)b2; (void)command_word;
			if (P::Phi2One(PHI2))
			{
				TriState T0 = core->wire.T0;
				TriState T1 = core->disp->getT1();
				TriState BR0 = AND(d[73], NOT(core->wire.n_PRDY));

				TriState JSR_5 = d[56];
				TriState RTS_5 = d[84];
				TriState BR2 = d[80];
				TriState BR3 = d[93];

				TriState pp = d[129];
				TriState ABS_2 = AND(d[83], NOT(pp));
				TriState JB = NOR3(d[94], d[95], d[96]);
				TriState n_PCH_DB = NOR(d[77], d[78]);

				// DB

				P::Set2(pch_db_latch1, n_PCH_DB, PHI2);
				TriState n_PCL_DB = pcl_db_latch1.nget();
				PC_DB = NAND(n_PCL_DB, n_PCH_DB);
				P::Set2(pcl_db_latch2, n_PCL_DB, PHI2);
				P::Set2(pch_db_latch2, n_PCH_DB, PHI2);

				// ADL

				TriState n1[5]{};
				n1[0] = T1;
				n1[1] = JSR_5;
				n1[2] = ABS_2;
				n1[3] = NOR(NOR(JB, nready_latch.get()), NOT(T0));
				n1[4] = BR2;
				TriState n_PCL_ADL = NOR5(n1);
				P::Set2(pcl_adl_latch, n_PCL_ADL, PHI2);

				TriState n2[4]{};
				n2[0] = NOT(n_PCL_ADL);
				n2[1] = RTS_5;
				n2[2] = T0;
				n2[3] = AND(NOT(nready_latch.get()), BR3);
				n_ADL_PCL = NOR4(n2);
				P::Set2(pcl_pcl_latch, NOT(n_ADL_PCL), PHI2);
				P::Set2(adl_pcl_latch, n_ADL_PCL, PHI2);

				// ADH

				TriState DL_PCH = NOR(NOT(T0), JB);
				TriState n_PCH_ADH = NOR(NOR3(n_PCL_ADL, DL_PCH, BR0), BR3);
				P::Set2(pch_adh_latch, n_PCH_ADH, PHI2);

				TriState n3[6]{};
				n3[0] = RTS_5;
				n3[1] = ABS_2;
				n3[2] = T0;
				n3[3] = T1;
				n3[4] = BR2;
				n3[5] = BR3;
				TriState n_ADH_PCH = NOR6(n3);
				P::Set2(adh_pch_latch, n_ADH_PCH, PHI2);
				TriState n_PCH_PCH = NOT(n_ADH_PCH);
				P::Set2(pch_pch_latch, n_PCH_PCH, PHI2);
			}
			else
			{
				// All you have to do during PHI1 is to update these 2 latches.

				P::Set1(nready_latch, n_ready, PHI1);
				P::Set1(pcl_db_latch1, NOR(pch_db_latch1.get(), n_ready), PHI1);
			}

			// Outputs

			core->cmd.PCL_DB = pcl_db_latch2.nget();
			core->cmd.PCH_DB = pch_db_latch2.nget();
			core->cmd.PCL_ADL = pcl_adl_latch.nget();
			core->cmd.PCH_ADH = pch_adh_latch.nget();

			if (P::Phi2One(PHI2))
			{
				core->cmd.PCL_PCL = 0;
				core->cmd.ADL_PCL = 0;
				core->cmd.ADH_PCH = 0;
				core->cmd.PCH_PCH = 0;
			}
			else
			{
				core->cmd.PCL_PCL = NOT(pcl_pcl_latch.get());
				core->cmd.ADL_PCL = NOT(adl_pcl_latch.get());
				core->cmd.ADH_PCH = NOT(adh_pch_latch.get());
				core->cmd.PCH_PCH = NOT(pch_pch_latch.get());
			}

			// For dispatcher

			core->wire.PC_DB = PC_DB;
			core->wire.n_ADL_PCL = n_ADL_PCL;
		}
	}
	// Out-of-line specializations also serve the typed CPU execution path.
	template void PC_Control::simImpl<false, 0>(uint64_t&, uint64_t&, uint64_t&);
	template void PC_Control::simImpl<true, 0>(uint64_t&, uint64_t&, uint64_t&);
	template void PC_Control::simImpl<true, 1>(uint64_t&, uint64_t&, uint64_t&);
	template void PC_Control::simImpl<true, 2>(uint64_t&, uint64_t&, uint64_t&);

}
