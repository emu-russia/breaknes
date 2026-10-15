#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	void BusControl::sim()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked) simImpl<true>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
		else simImpl<false>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
	}

	template<bool Packed, int Phase> void BusControl::simImpl(uint64_t& command_word, uint64_t& b1, uint64_t& b2)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI2 = core->wire.PHI2;

		if constexpr (Packed)
		{
			// Bit-bank form of the latches (random_logic.h): the twelve PHI2 latches are cmd_latch2 bits at the
			// command bits they drive. The reference form's nready_latch is enabled by PHI1 inside this PHI2-only
			// block, so it is never written: cmd_latch1 bit L1_BUS_NREADY stays Zero and is read as such.
			if (P::Phi2One(PHI2))
			{
			TriState BR0 = AND(d[73], NOT(core->wire.n_PRDY));
			TriState _AND = d[D_AND];
			TriState RMW_T7 = core->wire.RMW_T7;

			TriState n_SBXY = d[D_nSBXY];

			TriState PGX = OR(d[D_PGX_STATIC], BR0);	// NAND(NOR(d71, d72), NOT(BR0))

			TriState Z_ADL0 = core->cmd.Z_ADL0 ? TriState::One : TriState::Zero;
			TriState ACRL2 = core->wire.ACRL2;

			TriState JB = d[D_JB];
			TriState DL_PCH = NOR(NOT(core->wire.T0), JB);

			TriState n_ready = core->wire.n_ready;

			// NOT(NOR6(d39..d43, AND(RMW_T6, d44)))
			TriState INC_SB = OR(d[D_INCSB_ANY], AND(core->wire.RMW_T6, d[44]));

			TriState BRK6E = core->wire.BRK6E;
			TriState T0 = core->wire.T0;
			TriState T1 = core->disp->getT1();
			TriState RMW_T6 = core->wire.RMW_T6;

			TriState n_DL_ADL = d[D_nDL_ADL];
			TriState BR2 = d[80];
			TriState BR3 = d[93];
			TriState T2 = d[28];
			TriState JSR_5 = d[56];
			TriState JMP_4 = d[101];
			TriState JSXY = d[D_JSXY];

			TriState IND = d[D_IND];

			TriState IMPLIED = d[D_IMPLIED];
			TriState ABS_2 = d[D_ABS_2];
			TriState imp_abs = NOR(NOR(ABS_2, T0), IMPLIED);

			// NOR6(RTS_5, ABS_2, T0, T1, BR2, BR3) == NOR3(NOT(NOR4(RTS_5, ABS_2, BR2, BR3)), T0, T1)
			TriState n_ADH_PCH = NOR3(NOT(d[D_NAP_STATIC]), T0, T1);
			TriState n_PCH_PCH = NOT(n_ADH_PCH);

			TriState n_SB_ADH = NOR(PGX, BR3);
			TriState SBA = NOR(n_SB_ADH, NAND(ACRL2, LatchNGet(b1, L1_BUS_NREADY)));

			// External address bus control

			TriState n_ADL_ABL = NAND(NOR(RMW_T6, RMW_T7), NOR(d[D_PGX_STATIC], n_ready));	// NOT(NOR(d71, d72)) == PGX_STATIC

			TriState n1[4];
			n1[0] = IND;
			n1[1] = T2;
			n1[2] = n_PCH_PCH;
			n1[3] = JSR_5;
			TriState n_ADH_ABH = NOR(Z_ADL0, AND(OR(SBA, NOR(n_ready, NOR4(n1))), NOT(BR3)));

			// ALU connection to SB, DB buses (AC/DB, SB/AC, AC/SB)

			TriState n_SB_AC = d[D_nSB_AC];

			// Control of the SB, DB and ADH internal buses (SB/DB, SB/ADH, 0/ADH0, 0/ADH17)

			// NOR4(n_SBXY, NOT(n_SB_AC), RMW_T7, _AND) == NOR(NOT(NOR3(n_SBXY, NOT(n_SB_AC), _AND)), RMW_T7)
			TriState n_ZTST = NOR(NOT(d[D_ZTST_STATIC]), RMW_T7);

			TriState sbdb[6]{};
			sbdb[0] = NOT(NAND(RMW_T6, d[55]));
			sbdb[1] = NOR(n_ZTST, _AND);
			sbdb[2] = d[67];
			sbdb[3] = T1;
			sbdb[4] = BR2;
			sbdb[5] = JSXY;
			TriState n_SB_DB = NOR6(sbdb);

			// External data bus control

			// NOT(NOR6(INC_SB, d45, BRK6E, d46, d47, d48)) == OR3(INC_SB, N2_ANY, BRK6E)
			TriState n3[5]{};
			n3[0] = BR2;
			n3[1] = imp_abs;
			n3[2] = OR3(INC_SB, d[D_N2_ANY], BRK6E);
			n3[3] = JMP_4;
			n3[4] = RMW_T6;
			TriState n_DL_DB = NOR5(n3);

			const uint64_t w2 = LatchBit(n_ADL_ABL, L2_ADL_ABL) | LatchBit(n_ADH_ABH, L2_ADH_ABH) |
				LatchBit(n_SB_AC, L2_SB_AC) | LatchBit(d[D_nAC_SB], L2_AC_SB) | LatchBit(d[D_AC_DB_IN], L2_AC_DB) |
				LatchBit(n_DL_ADL, L2_Z_ADH0) | LatchBit(d[D_Z_ADH17_IN], L2_Z_ADH17) |
				LatchBit(n_SB_DB, L2_SB_DB) | LatchBit(n_SB_ADH, L2_SB_ADH) |
				LatchBit(NOR(DL_PCH, IND), L2_DL_ADH) | LatchBit(n_DL_ADL, L2_DL_ADL) | LatchBit(n_DL_DB, L2_DL_DB);
			b2 = (b2 & ~BusL2Mask) | w2;
			}

			// Outputs: every command is the latch's nget(); AC_DB, SB_AC and AC_SB are NOR(latch, PHI2), which is
			// NOT(latch) while bit 0 of PHI2 is clear (Zero, and the X pad) and Zero while it is set (One, Z).

			constexpr uint64_t ac = LatchMask(L2_SB_AC) | LatchMask(L2_AC_SB) | LatchMask(L2_AC_DB);
			const uint64_t nor_phi2 = P::Generic ? ((PHI2 & 1) ? 0 : ac) : (P::Is2 ? 0 : ac);
			const uint64_t commands = ~b2 & ((BusL2Mask & ~ac) | nor_phi2);
			command_word = (command_word & ~BusL2Mask) | commands;
		}
		else
		{
			(void)b1; (void)b2; (void)command_word;
			if (P::Phi2One(PHI2))
			{
				TriState BR0 = AND(d[73], NOT(core->wire.n_PRDY));
				TriState _AND = NOT(NOR(d[69], d[70]));
				TriState RMW_T7 = core->wire.RMW_T7;

				TriState n_SB_X = NOR3(d[14], d[15], d[16]);
				TriState n_SB_Y = NOR3(d[18], d[19], d[20]);
				TriState n_SBXY = NAND(n_SB_X, n_SB_Y);

				TriState PGX = NAND(NOR(d[71], d[72]), NOT(BR0));

				TriState PHI1 = core->wire.PHI1;
				TriState STOR = NOR(NOR5(d[111],d[122],d[123],d[124],d[125]),NOT(d[97]));
				TriState STXY = NOR(AND(STOR, d[0]), AND(STOR, d[12]));
				TriState Z_ADL0 = core->cmd.Z_ADL0 ? TriState::One : TriState::Zero;
				TriState ACRL2 = core->wire.ACRL2;
			
				TriState JB = NOR3(d[94], d[95], d[96]);
				TriState DL_PCH = NOR(NOT(core->wire.T0), JB);

				TriState n_ready = core->wire.n_ready;

				TriState incsb[6]{};
				incsb[0] = d[39];
				incsb[1] = d[40];
				incsb[2] = d[41];
				incsb[3] = d[42];
				incsb[4] = d[43];
				incsb[5] = AND(core->wire.RMW_T6, d[44]);
				TriState INC_SB = NOT(NOR6(incsb));

				TriState BRK6E = core->wire.BRK6E;
				TriState T0 = core->wire.T0;
				TriState T1 = core->disp->getT1();
				TriState RMW_T6 = core->wire.RMW_T6;
				TriState IR0 = core->ir->IROut & 1 ? TriState::One : TriState::Zero;

				TriState n_DL_ADL = NOR(d[81], d[82]);
				TriState RTS_5 = d[84];
				TriState BR2 = d[80];
				TriState BR3 = d[93];
				TriState T2 = d[28];
				TriState JSR2 = d[48];
				TriState JSR_5 = d[56];
				TriState JMP_4 = d[101];
				TriState pp = d[129];
				TriState JSXY = NAND(NOT(JSR2), STXY);

				TriState ind[4]{};
				ind[0] = d[89];
				ind[1] = AND(d[90], NOT(pp));
				ind[2] = d[91];
				ind[3] = RTS_5;
				TriState IND = NOT(NOR4(ind));

				TriState IMPLIED = AND(d[128], NOT(pp));
				IMPLIED = AND(IMPLIED, NOT(IR0));
				TriState ABS_2 = AND(d[83], NOT(pp));
				TriState imp_abs = NOR(NOR(ABS_2, T0), IMPLIED);

				TriState nap[6]{};
				nap[0] = RTS_5;
				nap[1] = ABS_2;
				nap[2] = T0;
				nap[3] = T1;
				nap[4] = BR2;
				nap[5] = BR3;
				TriState n_ADH_PCH = NOR6(nap);
				TriState n_PCH_PCH = NOT(n_ADH_PCH);

				P::Set1(nready_latch, n_ready, PHI1);
				TriState n_SB_ADH = NOR(PGX, BR3);
				TriState SBA = NOR(n_SB_ADH, NAND(ACRL2, nready_latch.nget()));

				// External address bus control

				TriState n_ADL_ABL = NAND(NOR(RMW_T6, RMW_T7), NOR(NOT(NOR(d[71], d[72])), n_ready));
				P::Set2(adl_abl_latch, n_ADL_ABL, PHI2);

				TriState n1[4];
				n1[0] = IND;
				n1[1] = T2;
				n1[2] = n_PCH_PCH;
				n1[3] = JSR_5;
				TriState n_ADH_ABH = NOR(Z_ADL0, AND(OR(SBA, NOR(n_ready, NOR4(n1))), NOT(BR3)));
				P::Set2(adh_abh_latch, n_ADH_ABH, PHI2);

				// ALU connection to SB, DB buses (AC/DB, SB/AC, AC/SB)

				TriState sbac[7]{};
				sbac[0] = d[58];
				sbac[1] = d[59];
				sbac[2] = d[60];
				sbac[3] = d[61];
				sbac[4] = d[62];
				sbac[5] = d[63];
				sbac[6] = d[64];
				TriState n_SB_AC = NOR7(sbac);
				P::Set2(sb_ac_latch, n_SB_AC, PHI2);

				TriState acsb[5]{};
				acsb[0] = AND(NOT(d[64]), d[65]);
				acsb[1] = d[66];
				acsb[2] = d[67];
				acsb[3] = d[68];
				acsb[4] = _AND;
				TriState n_AC_SB = NOR5(acsb);
				P::Set2(ac_sb_latch, n_AC_SB, PHI2);

				P::Set2(ac_db_latch, NOR(d[74], AND(d[79], STOR)), PHI2);

				// Control of the SB, DB and ADH internal buses (SB/DB, SB/ADH, 0/ADH0, 0/ADH17)

				P::Set2(z_adh0_latch, n_DL_ADL, PHI2);
				P::Set2(z_adh17_latch, NOR(d[57], NOT(n_DL_ADL)), PHI2);

				TriState ztst[4]{};
				ztst[0] = n_SBXY;
				ztst[1] = NOT(n_SB_AC);
				ztst[2] = RMW_T7;
				ztst[3] = _AND;
				TriState n_ZTST = NOR4(ztst);

				TriState sbdb[6]{};
				sbdb[0] = NOT(NAND(RMW_T6, d[55]));
				sbdb[1] = NOR(n_ZTST, _AND);
				sbdb[2] = d[67];
				sbdb[3] = T1;
				sbdb[4] = BR2;
				sbdb[5] = JSXY;
				TriState n_SB_DB = NOR6(sbdb);
				P::Set2(sb_db_latch, n_SB_DB, PHI2);

				P::Set2(sb_adh_latch, n_SB_ADH, PHI2);

				// External data bus control

				P::Set2(dl_adh_latch, NOR(DL_PCH, IND), PHI2);
				P::Set2(dl_adl_latch, n_DL_ADL, PHI2);

				TriState n2[6]{};
				n2[0] = INC_SB;
				n2[1] = d[45];
				n2[2] = BRK6E;
				n2[3] = d[46];
				n2[4] = d[47];
				n2[5] = JSR2;

				TriState n3[5]{};
				n3[0] = BR2;
				n3[1] = imp_abs;
				n3[2] = NOT(NOR6(n2));
				n3[3] = JMP_4;
				n3[4] = RMW_T6;
				TriState n_DL_DB = NOR5(n3);
				P::Set2(dl_db_latch, n_DL_DB, PHI2);
		
			}

			// Outputs

			core->cmd.ADL_ABL = adl_abl_latch.nget();
			core->cmd.ADH_ABH = adh_abh_latch.nget();

			core->cmd.AC_DB = P::NorPhi2(ac_db_latch.get(), PHI2);
			core->cmd.SB_AC = P::NorPhi2(sb_ac_latch.get(), PHI2);
			core->cmd.AC_SB = P::NorPhi2(ac_sb_latch.get(), PHI2);

			core->cmd.SB_DB = sb_db_latch.nget();
			core->cmd.SB_ADH = sb_adh_latch.nget();
			core->cmd.Z_ADH0 = z_adh0_latch.nget();
			core->cmd.Z_ADH17 = z_adh17_latch.nget();

			core->cmd.DL_ADL = dl_adl_latch.nget();
			core->cmd.DL_ADH = dl_adh_latch.nget();
			core->cmd.DL_DB = dl_db_latch.nget();
		}
	}
	// Out-of-line specializations also serve the typed CPU execution path.
	template void BusControl::simImpl<false, 0>(uint64_t&, uint64_t&, uint64_t&);
	template void BusControl::simImpl<true, 0>(uint64_t&, uint64_t&, uint64_t&);
	template void BusControl::simImpl<true, 1>(uint64_t&, uint64_t&, uint64_t&);
	template void BusControl::simImpl<true, 2>(uint64_t&, uint64_t&, uint64_t&);

}
