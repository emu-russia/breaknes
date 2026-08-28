#include "pch.h"

#include "MMC1.h"

// The "fast" approach (without BaseLogicLib) is used.

namespace Chips
{
	MMC1::MMC1()
	{
		// Power-up state: the control register is $0C (PRG mode 3, CHR 8K),
		// which puts the reset vector in the fixed last bank at $C000.
		reg[0].bitval = 0x0C;
	}

	MMC1::~MMC1()
	{
	}

	void MMC1::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[])
	{
		// Get signals from the inputs

		int m2 = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::M2]);
		int delayed_m2 = m2;
		int a13 = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::CPU_A13]);
		int a14 = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::CPU_A14]);
		int d0 = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::CPU_D0]);
		int d7 = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::CPU_D7]);
		int rnw = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::CPU_RnW]);
		int n_romsel = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::nROMSEL]);
		int ppu_a10 = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::PPU_A10]);
		int ppu_a11 = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::PPU_A11]);
		int ppu_a12 = BaseLogic::FromByte(inputs[(size_t)MMC1_Input::PPU_A12]);

		// Div Clock
		//
		// The write strobe (nROMSEL low) is only observable while M2 is high: the
		// board model derives /ROMSEL as !(A15 & M2) with no decode delay, so it
		// deasserts exactly when M2 falls (on the real board it lags M2 by the
		// ~33ns decode delay that PRG RAM decoders compensate for, so it is still
		// low at the falling edge where the die samples). The D7 reset data is
		// sampled while M2 is low, where the CPU data bus has settled; the divider
		// run latch is combinational so that a spurious M2-high sample (the bus is
		// still floating at the M2 posedge) is corrected once the data settles.

		if (m2) {
			div_clock_dff = !(rnw || n_romsel);
		}
		if (!m2) {
			// The CPU data bus has settled by the M2 low phase: sample the D7
			// reset data there and combine it with the write strobe that was
			// sampled while M2 was high.
			d7_settled = d7;
			div_reset_dff = !d7_settled || !div_clock_dff;
		}

		int div_ck = div_clock_dff && !m2;

		// Divider: a 3-bit ripple counter (div_dff[1] = LSB, div_dff[3] = MSB) clocked by
		// div_ck (the falling edge of M2 while a write is in progress). The transfer strobe
		// (decoder_disabled == 0) fires when the count reaches 5 (101). div_dff[0] is the
		// divider run latch (w36 in the deroute): it is asynchronously cleared by the D7
		// reset write (div_reset_dff == 0) and held at 0 by the transfer itself, so the
		// counter rests at 0 after a transfer until the following write re-arms it.

		int decoder_disabled = !(div_dff[1] && !div_dff[2] && div_dff[3]);

		div_dff[0] = div_reset_dff == 0 ? 0 : decoder_disabled;

		if (div_dff[0] == 0) {
			// Counter held at 0 (async reset of the ripple flops in the die).
			div_dff[1] = 0;
			div_dff[2] = 0;
			div_dff[3] = 0;
			prev_div_dff[1] = 0;
			prev_div_dff[2] = 0;
			prev_div_dff[3] = 0;
		} else {
			if (posedge(prev_div_ck, div_ck)) {
				div_dff[1] = !div_dff[1];
			}
			if (posedge(!prev_div_dff[1], !div_dff[1])) {
				div_dff[2] = !div_dff[2];
			}
			if (posedge(!prev_div_dff[2], !div_dff[2])) {
				div_dff[3] = !div_dff[3];
			}
			prev_div_dff[1] = div_dff[1];
			prev_div_dff[2] = div_dff[2];
			prev_div_dff[3] = div_dff[3];
		}

		// CPU D0 In

		// D0 is latched at the FALLING edge of M2 (like the deroute's w49):
		// the CPU data bus still holds the written value at that point.
		if (posedge(!prev_m2, !m2)) {
			cpu_d0_dff = d0;
		}

		// Reg Decoder

		if (posedge(prev_div_ck, div_ck)) {
			decoder_dffs = a13 | (a14 << 1);
		}

		int reg_enable = -1;
		if (!decoder_disabled) {
			reg_enable = decoder_dffs;
		}

		// Shift Reg

		if (posedge(prev_div_ck, div_ck)) {
			shifter_dffs = shifter_dffs >> 1;
			shifter_dffs |= (cpu_d0_dff << 4);
			shifter_dffs &= 0x1f;		// just in case
		}

		// Regs 0-3

		if (reg_enable >= 0) {
			reg[reg_enable].bitval = shifter_dffs;
		}
		// Special processing for bits 2 and 3 of the control register. These bits are set when the divider is reset from the outside.
		if (div_reset_dff == 0) {
			reg[0].b2 = 1;
			reg[0].b3 = 1;
		}

		// CHR Bank Switch

		outputs[(size_t)MMC1_Output::CHR_A12] = BaseLogic::FromByte( (1 && (reg[2].b0 || !reg[0].b4) && ppu_a12) || (!ppu_a12 && reg[0].b4 && reg[1].b0) );	// 33-aon
		outputs[(size_t)MMC1_Output::CHR_A13] = BaseLogic::FromByte( ((!reg[0].b4 || !ppu_a12) && reg[1].b1) || (reg[0].b4 && ppu_a12 && reg[2].b1));		// 22-aon
		outputs[(size_t)MMC1_Output::CHR_A14] = BaseLogic::FromByte( ((!reg[0].b4 || !ppu_a12) && reg[1].b2) || (reg[0].b4 && ppu_a12 && reg[2].b2));		// 22-aon
		outputs[(size_t)MMC1_Output::CHR_A15] = BaseLogic::FromByte( ((!reg[0].b4 || !ppu_a12) && reg[1].b3) || ((reg[0].b4 && ppu_a12) && reg[2].b3));		// 22-aon
		outputs[(size_t)MMC1_Output::CHR_A16] = BaseLogic::FromByte( ((!reg[0].b4 || !ppu_a12) && reg[1].b4) || ((reg[0].b4 && ppu_a12) && reg[2].b4));		// 22-aon

		// PRG Bank Switch
		//
		// Standard MMC1 PRG modes (the deroute is the early "letterless" revision
		// whose modes 2/3 map the $8000 window to the last-32K region instead of
		// the switchable bank; the standard table below is what games are written
		// against and matches the reset behavior they expect):
		//   mode 0/1 (reg0.b2 = 0): 32K window at (reg3 & 0x0E) << 14 (bit 0 ignored)
		//   mode 2 (b2=1, b3=0):    $8000 = bank 0 fixed, $C000 = reg3 & 0x0F
		//   mode 3 (b2=1, b3=1):    $8000 = reg3 & 0x0F, $C000 = last bank (all ones)

		int prg_a14 = !reg[0].b2 ? a14 : (reg[0].b3 ? (reg[3].b0 || a14) : (reg[3].b0 && a14));
		int prg_a15 = !reg[0].b2 ? reg[3].b1 : (reg[0].b3 ? (reg[3].b1 || a14) : (reg[3].b1 && a14));
		int prg_a16 = !reg[0].b2 ? reg[3].b2 : (reg[0].b3 ? (reg[3].b2 || a14) : (reg[3].b2 && a14));
		int prg_a17 = !reg[0].b2 ? reg[3].b3 : (reg[0].b3 ? (reg[3].b3 || a14) : (reg[3].b3 && a14));

		outputs[(size_t)MMC1_Output::PRG_A14] = BaseLogic::FromByte(prg_a14);
		outputs[(size_t)MMC1_Output::PRG_A15] = BaseLogic::FromByte(prg_a15);
		outputs[(size_t)MMC1_Output::PRG_A16] = BaseLogic::FromByte(prg_a16);
		outputs[(size_t)MMC1_Output::PRG_A17] = BaseLogic::FromByte(prg_a17);

		// CIRAM A10 Line

		outputs[(size_t)MMC1_Output::VRAM_A10] = BaseLogic::FromByte((ppu_a11 && reg[0].b0 && reg[0].b1) || (!reg[0].b0 && reg[0].b1 && ppu_a10) || (1 && reg[0].b0 && !reg[0].b1));		// 333-aon

		// SRAM Chip Select

		outputs[(size_t)MMC1_Output::SRAM_CE] = BaseLogic::FromByte(n_romsel && a13 && a14 && delayed_m2 && m2);

		// PRG Chip Select

		outputs[(size_t)MMC1_Output::PRG_nCE] = BaseLogic::FromByte(n_romsel || !rnw);

		// Freeze edge

		prev_m2 = m2;
		prev_div_ck = div_ck;
		prev_reg0_enable = reg_enable == 0 ? 1 : 0;
	}

	bool MMC1::posedge(int prev, int cur)
	{
		return (prev == 0 && cur);
	}

	size_t MMC1::Dbg_GetPRGAddress(size_t cpu_addr)
	{
		// Mirror the PRG bank switching formulas from sim() using the current register state.

		int a14 = (cpu_addr >> 14) & 1;

		int prg_a14 = !reg[0].b2 ? a14 : (reg[0].b3 ? (reg[3].b0 || a14) : (reg[3].b0 && a14));
		int prg_a15 = !reg[0].b2 ? reg[3].b1 : (reg[0].b3 ? (reg[3].b1 || a14) : (reg[3].b1 && a14));
		int prg_a16 = !reg[0].b2 ? reg[3].b2 : (reg[0].b3 ? (reg[3].b2 || a14) : (reg[3].b2 && a14));
		int prg_a17 = !reg[0].b2 ? reg[3].b3 : (reg[0].b3 ? (reg[3].b3 || a14) : (reg[3].b3 && a14));

		return (cpu_addr & 0x3fff) |
			((size_t)prg_a14 << 14) |
			((size_t)prg_a15 << 15) |
			((size_t)prg_a16 << 16) |
			((size_t)prg_a17 << 17);
	}
}

