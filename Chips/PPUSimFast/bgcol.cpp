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

	void BGCol::sim()
	{
		sim_Control();
#if PPUSIM_FAST_BG_PHASE
		sim_BGC0();
		sim_BGC1();
		sim_BGC2();
		sim_BGC3();
		sim_Output();
#else
		sim_Packed();
#endif
	}

	void BGCol::sim_Control()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState F_TA = ppu->fsm.FTA;
		TriState F_TB = ppu->fsm.FTB;
		TriState n_FO = ppu->fsm.nFO;
		TriState F_AT = ppu->fsm.FAT;
		TriState THO1 = ppu->wire.THO[1];

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

	void BGCol::sim_Packed()
	{
		const uint8_t data = ppu->PD;
		const unsigned fine_x = Pack3(ppu->wire.FH);
		if (PD_SR == TriState::One) captured_pattern = data;
		if (PD_SEL == TriState::One) captured_attribute = data;
		const uint8_t tile0 = BGC0_SR1.sim_Packed(Reverse(static_cast<uint8_t>(~captured_pattern)), TriState::One, SRLOAD, STEP, NEXT);
		const uint8_t pixels0 = BGC0_SR2.sim_Packed(0, static_cast<TriState>(tile0 & 1), TriState::Zero, STEP2, NEXT);
		const uint8_t tile1 = BGC1_SR1.sim_Packed(Reverse(data), TriState::One, SRLOAD, STEP, NEXT);
		const uint8_t pixels1 = BGC1_SR2.sim_Packed(0, static_cast<TriState>(tile1 & 1), TriState::Zero, STEP2, NEXT);
		const unsigned quadrant = (H01 == TriState::One ? 1U : 0U) | (ppu->wire.TVO[1] == TriState::One ? 2U : 0U);
		TriState shift2, shift3;
		BGC2_SRBit1.sim(TriState::Z, static_cast<TriState>(((captured_attribute >> (2 * quadrant)) & 1) ^ 1), SRLOAD, TriState::Zero, NEXT, shift2);
		BGC3_SRBit1.sim(TriState::Z, static_cast<TriState>(((captured_attribute >> (2 * quadrant + 1)) & 1) ^ 1), SRLOAD, TriState::Zero, NEXT, shift3);
		const uint8_t pixels2 = BGC2_SR1.sim_Packed(0, shift2, TriState::Zero, STEP2, NEXT);
		const uint8_t pixels3 = BGC3_SR1.sim_Packed(0, shift3, TriState::Zero, STEP2, NEXT);
		if (ppu->wire.PCLK == TriState::One)
		{
			clpb_latch.set(ppu->wire.n_CLPB, TriState::One);
			pixel_output = static_cast<uint8_t>(~pixel_input) & 0x0f;
		}
		else
		{
			pixel_input = static_cast<uint8_t>((((pixels0 >> fine_x) & 1) ^ 1) |
				(((pixels1 >> fine_x) & 1) << 1) | ((((pixels2 >> fine_x) & 1) ^ 1) << 2) |
				((((pixels3 >> fine_x) & 1) ^ 1) << 3));
		}
		const uint8_t pixel = clpb_latch.get() == TriState::One ? static_cast<uint8_t>(~pixel_output) & 0x0f : 0;
		for (size_t n = 0; n < 4; n++) ppu->wire.BGC[n] = static_cast<TriState>((pixel >> n) & 1);
	}
}
