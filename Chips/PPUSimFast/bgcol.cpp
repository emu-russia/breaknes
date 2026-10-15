// Background Color (BG COL)

#include "pch.h"

using namespace BaseLogic;

#ifndef PPUSIM_FAST_BG_PHASE
#define PPUSIM_FAST_BG_PHASE 0
#endif

namespace PPUSimFast
{
	static uint8_t Reverse(uint8_t value)
	{
		value = static_cast<uint8_t>(((value & 0x55) << 1) | ((value >> 1) & 0x55));
		value = static_cast<uint8_t>(((value & 0x33) << 2) | ((value >> 2) & 0x33));
		return static_cast<uint8_t>((value << 4) | (value >> 4));
	}

	BGCol::BGCol(PPU* parent)
	{
		ppu = parent;
	}

	BGCol::~BGCol()
	{
	}

	template<int Phase> void BGCol::sim(const EdgeContextT<Phase>& c)
	{
#if PPUSIM_FAST_BG_PHASE
		Unpack3(ppu->data_reader->sccx->get_FH(), ppu->wire.FH);
		ppu->wire.THO[1] = static_cast<TriState>((ppu->data_reader->tilecnt->get_TH() >> 1) & 1);
		ppu->wire.TVO[1] = static_cast<TriState>((ppu->data_reader->tilecnt->get_TV() >> 1) & 1);
		sim_Control(c);
		sim_BGC0(); sim_BGC1(); sim_BGC2(); sim_BGC3(); sim_Output();
#else
		if (c.PCLK == TriState::One)
		{
			fat_latch.set(NOT(c.FAT), TriState::One);
			tho1_latch.set(static_cast<TriState>((ppu->data_reader->tilecnt->get_TH() >> 1) & 1), TriState::One);
		}
		else sim_Control(c);
		sim_Packed(c);
#endif
	}

	template<int Phase> void BGCol::sim_Control(const EdgeContextT<Phase>& c)
	{
		TriState PCLK = c.PCLK;
		TriState n_PCLK = c.n_PCLK;
		TriState H0_DD = c.H0_Dash2;
		TriState F_TA = c.FTA;
		TriState F_TB = c.FTB;
		TriState n_FO = c.nFO;
		TriState F_AT = c.FAT;
		TriState THO1 = static_cast<TriState>((ppu->data_reader->tilecnt->get_TH() >> 1) & 1);

		fat_latch.set(NOT(F_AT), PCLK);
		tho1_latch.set(THO1, PCLK);

		auto nH0DD = NOT(H0_DD);

		PD_SR = NOR3(NOT(F_TA), nH0DD, PCLK);
		SRLOAD = NOR3(NOT(F_TB), nH0DD, PCLK);
		STEP = NOR3(PCLK, NOR(NOT(F_TB), nH0DD), NOT(n_FO));
		STEP2 = NOR(PCLK, NOT(n_FO));
		PD_SEL = NOR3(fat_latch.get(), PCLK, nH0DD);
		NEXT = NOR3(STEP, STEP2, n_PCLK);
		H01 = NOT(tho1_latch.nget());
	}

	void BGCol::sim_BGC0()
	{
		TriState sr_in[8]{};
		TriState sr_out1[8]{};
		TriState sr_out2[8]{};

		for (size_t n = 0; n < 8; n++)
		{
			BGC0_Latch[n].set(ppu->GetPDBit(n), PD_SR);
			sr_in[7 - n] = BGC0_Latch[n].nget();
		}

		BGC0_SR1.sim(sr_in, TriState::One, SRLOAD, STEP, NEXT, sr_out1);
		BGC0_SR2.sim(unused, sr_out1[0], TriState::Zero, STEP2, NEXT, sr_out2);
		n_BGC0_Out = MUX3(ppu->wire.FH, sr_out2);
	}

	void BGCol::sim_BGC1()
	{
		TriState sr_in[8]{};
		TriState sr_out1[8]{};
		TriState sr_out2[8]{};

		for (size_t n = 0; n < 8; n++)
		{
			sr_in[7 - n] = ppu->GetPDBit(n);
		}

		BGC1_SR1.sim(sr_in, TriState::One, SRLOAD, STEP, NEXT, sr_out1);
		BGC1_SR2.sim(unused, sr_out1[0], TriState::Zero, STEP2, NEXT, sr_out2);
		BGC1_Out = MUX3(ppu->wire.FH, sr_out2);
	}

	void BGCol::sim_BGC2()
	{
		pd_latch[0].set(ppu->GetPDBit(0), PD_SEL);
		pd_latch[2].set(ppu->GetPDBit(2), PD_SEL);
		pd_latch[4].set(ppu->GetPDBit(4), PD_SEL);
		pd_latch[6].set(ppu->GetPDBit(6), PD_SEL);

		TriState mux_sel[2]{};
		mux_sel[0] = H01;
		mux_sel[1] = ppu->wire.TVO[1];

		TriState mux_in[4]{};
		mux_in[0] = pd_latch[0].nget();
		mux_in[1] = pd_latch[2].nget();
		mux_in[2] = pd_latch[4].nget();
		mux_in[3] = pd_latch[6].nget();

		TriState shift_out;
		BGC2_SRBit1.sim(TriState::Z, MUX2(mux_sel, mux_in), SRLOAD, TriState::Zero, NEXT, shift_out);

		TriState sr_out[8]{};
		BGC2_SR1.sim(unused, shift_out, TriState::Zero, STEP2, NEXT, sr_out);

		n_BGC2_Out = MUX3(ppu->wire.FH, sr_out);
	}

	void BGCol::sim_BGC3()
	{
		pd_latch[1].set(ppu->GetPDBit(1), PD_SEL);
		pd_latch[3].set(ppu->GetPDBit(3), PD_SEL);
		pd_latch[5].set(ppu->GetPDBit(5), PD_SEL);
		pd_latch[7].set(ppu->GetPDBit(7), PD_SEL);

		TriState mux_sel[2]{};
		mux_sel[0] = H01;
		mux_sel[1] = ppu->wire.TVO[1];

		TriState mux_in[4]{};
		mux_in[0] = pd_latch[1].nget();
		mux_in[1] = pd_latch[3].nget();
		mux_in[2] = pd_latch[5].nget();
		mux_in[3] = pd_latch[7].nget();

		TriState shift_out;
		BGC3_SRBit1.sim(TriState::Z, MUX2(mux_sel, mux_in), SRLOAD, TriState::Zero, NEXT, shift_out);

		TriState sr_out[8]{};
		BGC3_SR1.sim(unused, shift_out, TriState::Zero, STEP2, NEXT, sr_out);

		n_BGC3_Out = MUX3(ppu->wire.FH, sr_out);
	}

	void BGCol::sim_Output()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_CLPB = ppu->wire.n_CLPB;

		clpb_latch.set(n_CLPB, PCLK);
		auto clip = clpb_latch.nget();

		bgc0_latch[0].set(NOT(n_BGC0_Out), n_PCLK);
		bgc0_latch[1].set(bgc0_latch[0].nget(), PCLK);

		bgc1_latch[0].set(BGC1_Out, n_PCLK);
		bgc1_latch[1].set(bgc1_latch[0].nget(), PCLK);

		bgc2_latch[0].set(NOT(n_BGC2_Out), n_PCLK);
		bgc2_latch[1].set(bgc2_latch[0].nget(), PCLK);

		bgc3_latch[0].set(NOT(n_BGC3_Out), n_PCLK);
		bgc3_latch[1].set(bgc3_latch[0].nget(), PCLK);

		ppu->wire.BGC[0] = NOR(bgc0_latch[1].get(), clip);
		ppu->wire.BGC[1] = NOR(bgc1_latch[1].get(), clip);
		ppu->wire.BGC[2] = NOR(bgc2_latch[1].get(), clip);
		ppu->wire.BGC[3] = NOR(bgc3_latch[1].get(), clip);
		visible_pixel = PackNibble(ppu->wire.BGC);
	}

	void BGC_SRBit::sim(TriState shift_in, TriState val_in,
		TriState Load, TriState Step, TriState Next,
		TriState& shift_out)
	{
		in_latch.set(MUX(Step, MUX(Load, TriState::Z, val_in), shift_in), TriState::One);
		out_latch.set(in_latch.nget(), Next);
		shift_out = out_latch.nget();
	}

	uint8_t BGC_SR8::sim_Packed(uint8_t value, TriState sin, TriState Load, TriState Step, TriState Next)
	{
		if ((Step & 1) != 0)
			input = Next == TriState::One ? (sin == TriState::One ? 0xff : 0) :
				static_cast<uint8_t>((static_cast<uint8_t>(~output) >> 1) | (sin == TriState::One ? 0x80 : 0));
		else if ((Load & 1) != 0) input = value;
		if (Next == TriState::One) output = static_cast<uint8_t>(~input);
		return static_cast<uint8_t>(~output);
	}

	void BGC_SR8::sim(TriState val[8], TriState sin, TriState Load, TriState Step, TriState Next, TriState sout[8])
	{
		Unpack(sim_Packed(Pack(val), sin, Load, Step, Next), sout);
	}

	template<int Phase> void BGCol::sim_Packed(const EdgeContextT<Phase>& c)
	{
		// The controller makes STEP/LOAD mutually exclusive, and NEXT is
		// exactly the high phase. Prepared and published banks stay distinct,
		// including repeated high/low invocations caused by reset.
		const bool high = c.PCLK == TriState::One;
		const unsigned fine_x = ppu->data_reader->sccx->get_FH();
		if (high)
		{
			bank_output = ~bank_input;
			attribute_output = uint8_t(~attribute_input) & 3;
			clpb_latch.set(c.n_CLPB, TriState::One);
			pixel_output = uint8_t(~pixel_input) & 15;
		}
		else
		{
			const uint8_t data = ppu->PD;
			if (PD_SR == TriState::One) captured_pattern = data;
			if (PD_SEL == TriState::One) captured_attribute = data;
			const unsigned quadrant = (H01 == TriState::One ? 1U : 0U) |
				(ppu->data_reader->tilecnt->get_TV() & 2);
			if (SRLOAD == TriState::One)
			{
				bank_input = (bank_input & ~UINT64_C(0x0000000000ff00ff)) |
					Reverse(uint8_t(~captured_pattern)) | (uint64_t(Reverse(data)) << 16);
				attribute_input = uint8_t(~(captured_attribute >> (2 * quadrant))) & 3;
			}
			if (STEP == TriState::One)
			{
				const uint64_t shift = ((~bank_output >> 1) & UINT64_C(0x00000000007f007f)) | UINT64_C(0x0000000000800080);
				bank_input = (bank_input & ~UINT64_C(0x0000000000ff00ff)) | shift;
			}
			if (STEP2 == TriState::One)
			{
				const uint64_t old = ~bank_output;
				const uint64_t incoming = ((old & 1) << 15) | (((old >> 16) & 1) << 31) |
					(uint64_t((attribute_output & 1) ^ 1) << 39) | (uint64_t(((attribute_output >> 1) & 1) ^ 1) << 47);
				const uint64_t mask = UINT64_C(0x0000ffffff00ff00);
				bank_input = (bank_input & ~mask) | ((old >> 1) & UINT64_C(0x00007f7f7f007f00)) | incoming;
			}
			const uint64_t pixels = ~bank_output;
			pixel_input = uint8_t((((pixels >> (8 + fine_x)) & 1) ^ 1) |
				(((pixels >> (24 + fine_x)) & 1) << 1) |
				((((pixels >> (32 + fine_x)) & 1) ^ 1) << 2) |
				((((pixels >> (40 + fine_x)) & 1) ^ 1) << 3));
		}
		const uint8_t pixel = clpb_latch.get() == TriState::One ? uint8_t(~pixel_output) & 15 : 0;
		visible_pixel = pixel;
	}

	void BGCol::sim() { sim(CaptureEdge(*ppu)); }

	template void BGCol::sim<0>(const EdgeContextT<0>&);
	template void BGCol::sim<1>(const EdgeContextT<1>&);
}
