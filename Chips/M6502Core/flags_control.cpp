#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	FlagsControl::FlagsControl(M6502* parent)
	{
		core = parent;

		if (core->GetExecutionBackend() == ExecutionBackend::Reference)
		{
			temp_tab = new FlagsControl_TempWire[0x10000];
			size_t maxVal = 1ULL << 16;
			for (size_t n = 0; n < maxVal; n++)
			{
				uint8_t ir = n & 0xff;
				bool n_T0 = (n >> 8) & 1;
				bool n_T1X = (n >> 9) & 1;
				bool n_T2 = (n >> 10) & 1;
				bool n_T3 = (n >> 11) & 1;
				bool n_T4 = (n >> 12) & 1;
				bool n_T5 = (n >> 13) & 1;
				bool T5 = (n >> 14) & 1;
				bool T6 = (n >> 15) & 1;

				temp_tab[n] = PreCalc(ir, n_T0, n_T1X, n_T2, n_T3, n_T4, n_T5, T5, T6);
			}

		}

		prev_temp.bits = 0xff;
	}

	void FlagsControl::sim()
	{
		if (core->GetExecutionBackend() == ExecutionBackend::ExactPacked) simImpl<true>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
		else simImpl<false>(core->cmd.raw, core->cmd_latch1, core->cmd_latch2);
	}

	template<bool Packed, int Phase> void FlagsControl::simImpl(uint64_t& command_word, uint64_t& b1, uint64_t& b2)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		DecoderView<Packed> d(core->decoder_out,core->packed_decoder);
		TriState PHI2 = core->wire.PHI2;
		TriState n_ready = core->wire.n_ready;
		TriState DB_P;

		if constexpr (Packed)
		{
			// Bit-bank form of the latches (random_logic.h): the eleven PHI2 latches are cmd_latch2 bits, eight of
			// them at the command bits they drive. acrc_latch and pin_latch are read back inside the PHI2 block
			// right after their write; they then hold the descriptor's n_ARIT / n_PIN (whether or not the
			// reference form's unchanged-descriptor guard would have rewritten them).
			(void)b1;
			if (P::Phi2One(PHI2))
			{
				TriState RMW_T6 = core->wire.RMW_T6;
				TriState RMW_T7 = core->wire.RMW_T7;

				FlagsControl_TempWire temp{};
				temp.bits = core->packed_decoder->FlagControls(RMW_T6 != Zero,RMW_T7 != Zero);
				prev_temp.bits = temp.bits;

				// Latches

				const TriState n_POUT = temp.n_POUT ? TriState::One : TriState::Zero;
				const TriState n_ARIT = temp.n_ARIT ? TriState::One : TriState::Zero;
				const TriState n_PIN = temp.n_PIN ? TriState::One : TriState::Zero;
				DB_P = NOR(n_PIN, n_ready);
				const uint64_t w2 = LatchBit(n_POUT, L2_PDB) | LatchBit(n_ARIT, L2_ACRC) | LatchBit(n_PIN, L2_PIN) |
					LatchBit(NOT(d[108]), L2_IRI) | LatchBit(NOT(d[110]), L2_IRC) | LatchBit(NOT(d[120]), L2_IRD) | LatchBit(NOT(d[127]), L2_ZV) |
					LatchBit(NOR3(NOT(n_ARIT), temp.ZTST ? TriState::One : TriState::Zero, d[109]), L2_DBZ) |
					LatchBit(d[109], L2_DBN) |
					LatchBit(NOR(DB_P, temp.SR ? TriState::One : TriState::Zero), L2_DBC) |
					LatchBit(NOT(d[113]), L2_BIT);
				b2 = (b2 & ~FlagsL2Mask) | w2;
			}
			else
			{
				DB_P = NOR(LatchGet(b2, L2_PIN), n_ready);
			}

			// Outputs: P_DB, DBZ_Z, IR5_C, DB_C, ACR_C, IR5_D, IR5_I and Z_V are the latch's nget(); DB_P, DB_N and
			// DB_V are gates over the latches; AVR_V is the live decoder line.

			constexpr uint64_t ngets = LatchMask(L2_PDB) | LatchMask(L2_DBZ) | LatchMask(L2_IRC) | LatchMask(L2_DBC) |
				LatchMask(L2_ACRC) | LatchMask(L2_IRD) | LatchMask(L2_IRI) | LatchMask(L2_ZV);
			const TriState pin = LatchGet(b2, L2_PIN);
			const TriState DB_N = NOR(AND(LatchGet(b2, L2_DBZ), pin), LatchGet(b2, L2_DBN));
			const TriState DB_V = NAND(pin, LatchGet(b2, L2_BIT));
			const uint64_t commands = (~b2 & ngets) | (uint64_t(d[112]) << 57) | (uint64_t(DB_N) << 50) | (uint64_t(DB_P) << 48) | (uint64_t(DB_V) << 56);
			command_word = (command_word & ~UINT64_C(576320014815068160)) | commands;
		}
		else
		{
			(void)b1; (void)b2; (void)command_word;
			if (P::Phi2One(PHI2))
			{
				TriState RMW_T6 = core->wire.RMW_T6;
				TriState RMW_T7 = core->wire.RMW_T7;

				size_t n = 0;
				n |= core->ir->IROut;
				n |= ((size_t)core->TxBits << 8);
				n |= ((size_t)RMW_T6 << 14);
				n |= ((size_t)RMW_T7 << 15);

				FlagsControl_TempWire temp = temp_tab[n];

				// Latches

				if (prev_temp.bits != temp.bits)
				{
					P::Set2(pdb_latch, temp.n_POUT ? TriState::One : TriState::Zero, PHI2);
					P::Set2(acrc_latch, temp.n_ARIT ? TriState::One : TriState::Zero, PHI2);
					P::Set2(pin_latch, temp.n_PIN ? TriState::One : TriState::Zero, PHI2);
					prev_temp.bits = temp.bits;
				}

				P::Set2(iri_latch, NOT(d[108]), PHI2);
				P::Set2(irc_latch, NOT(d[110]), PHI2);
				P::Set2(ird_latch, NOT(d[120]), PHI2);
				P::Set2(zv_latch, NOT(d[127]), PHI2);
				P::Set2(dbz_latch, NOR3(acrc_latch.nget(), temp.ZTST ? TriState::One : TriState::Zero, d[109]), PHI2);
				P::Set2(dbn_latch, d[109], PHI2);
				DB_P = NOR(pin_latch.get(), n_ready);
				P::Set2(dbc_latch, NOR(DB_P, temp.SR ? TriState::One : TriState::Zero), PHI2);
				P::Set2(bit_latch, NOT(d[113]), PHI2);
			}
			else
			{
				DB_P = NOR(pin_latch.get(), n_ready);
			}

			// Outputs

			core->cmd.P_DB = pdb_latch.nget();
			core->cmd.IR5_I = iri_latch.nget();
			core->cmd.IR5_C = irc_latch.nget();
			core->cmd.IR5_D = ird_latch.nget();
			core->cmd.AVR_V = d[112];
			core->cmd.Z_V = zv_latch.nget();
			core->cmd.ACR_C = acrc_latch.nget();
			core->cmd.DBZ_Z = dbz_latch.nget();
			core->cmd.DB_N = NOR(AND(dbz_latch.get(), pin_latch.get()), dbn_latch.get());
			core->cmd.DB_P = DB_P;
			core->cmd.DB_C = dbc_latch.nget();
			core->cmd.DB_V = NAND(pin_latch.get(), bit_latch.get());
		}
	}

	FlagsControl_TempWire FlagsControl::PreCalc(uint8_t ir, bool n_T0, bool n_T1X, bool n_T2, bool n_T3, bool n_T4, bool n_T5, bool T5, bool T6)
	{
		TriState* d;
		DecoderInput decoder_in{};
		decoder_in.packed_bits = 0;
		FlagsControl_TempWire temp{};
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

		TriState sbac[7]{};
		sbac[0] = d[58];
		sbac[1] = d[59];
		sbac[2] = d[60];
		sbac[3] = d[61];
		sbac[4] = d[62];
		sbac[5] = d[63];
		sbac[6] = d[64];
		TriState n_SB_AC = NOR7(sbac);

		TriState _AND = NOT(NOR(d[69], d[70]));

		TriState n_SB_X = NOR3(d[14], d[15], d[16]);
		TriState n_SB_Y = NOR3(d[18], d[19], d[20]);
		TriState n_SBXY = NAND(n_SB_X, n_SB_Y);

		TriState ztst[4]{};
		ztst[0] = n_SBXY;
		ztst[1] = NOT(n_SB_AC);
		ztst[2] = T6 ? TriState::One : TriState::Zero;
		ztst[3] = _AND;
		TriState n_ZTST = NOR4(ztst);

		temp.ZTST = NOT(n_ZTST);
		temp.SR = NOT(NOR(d[75], AND(d[76], T5 ? TriState::One : TriState::Zero)));

		temp.n_POUT = NOR(d[98], d[99]);
		temp.n_PIN = NOR(d[114], d[115]);

		TriState n1[6]{};
		n1[0] = AND(d[107], T6 ? TriState::One : TriState::Zero);
		n1[1] = d[112];		// AVR/V
		n1[2] = d[116];
		n1[3] = d[117];
		n1[4] = d[118];
		n1[5] = d[119];
		temp.n_ARIT = NOR6(n1);

		return temp;
	}
	// Out-of-line specializations also serve the typed CPU execution path.
	template void FlagsControl::simImpl<false, 0>(uint64_t&, uint64_t&, uint64_t&);
	template void FlagsControl::simImpl<true, 0>(uint64_t&, uint64_t&, uint64_t&);
	template void FlagsControl::simImpl<true, 1>(uint64_t&, uint64_t&, uint64_t&);
	template void FlagsControl::simImpl<true, 2>(uint64_t&, uint64_t&, uint64_t&);

}
