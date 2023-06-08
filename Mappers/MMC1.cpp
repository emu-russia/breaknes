#include "pch.h"

// The "fast" approach (without BaseLogicLib) is used.

namespace Mappers
{
	MMC1::MMC1()
	{
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

		int rst_ck = !m2;
		int prev_rst_ck = !prev_m2;

		if (posedge(prev_rst_ck, rst_ck)) {
			div_clock_dff = !(rnw || n_romsel);
		}

		int div_ck = div_clock_dff && !m2;

		// Div Reset

		if (posedge(prev_rst_ck, rst_ck)) {
			div_reset_dff = !d7 || n_romsel || rnw;
		}

		// Divider (hmmm...)

		int decoder_disabled = !(div_dff[1] && !div_dff[2] && div_dff[3]);

		if (posedge(prev_m2, m2)) {
			div_dff[0] = div_reset_dff == 0 ? 0 : decoder_disabled;
		}
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

		// CPU D0 In

		if (posedge(prev_rst_ck, rst_ck)) {
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

		if (reg_enable > 0) {
			if (reg_enable == 0) {
				int extra_enable = div_reset_dff;
				reg[0].b0 = (shifter_dffs >> 0) & 1;
				reg[0].b1 = (shifter_dffs >> 1) & 1;
				// Special processing for bits 2 and 3 of the control register. Most likely related to PRG bank switching, e.g. not to switch yourself.
				if (posedge(prev_reg0_enable, 1) && extra_enable) {
					reg[0].b2 = (shifter_dffs >> 2) & 1;
					reg[0].b3 = (shifter_dffs >> 3) & 1;
				}
				reg[0].b4 = (shifter_dffs >> 4) & 1;
			}
			else {
				reg[reg_enable].bitval = shifter_dffs;
			}
		}

		// CHR Bank Switch

		outputs[(size_t)MMC1_Output::CHR_A12] = BaseLogic::FromByte( (1 && (reg[2].b0 || !reg[0].b4) && ppu_a12) || (!ppu_a12 && reg[0].b4 && reg[1].b0) );	// 33-aon
		outputs[(size_t)MMC1_Output::CHR_A13] = BaseLogic::FromByte( ((!reg[0].b4 || reg[1].b1) && (reg[0].b4 && ppu_a12)) || (reg[2].b1 && (reg[0].b4 && ppu_a12)));		// 22-aon
		outputs[(size_t)MMC1_Output::CHR_A14] = BaseLogic::FromByte( ((reg[2].b0 || !reg[0].b4) && reg[1].b2) || ((reg[0].b4 && ppu_a12) && reg[2].b2));		// 22-aon
		outputs[(size_t)MMC1_Output::CHR_A15] = BaseLogic::FromByte( ((!reg[0].b4 || !ppu_a12) && reg[1].b3) || ((reg[0].b4 && ppu_a12) && reg[2].b3));		// 22-aon
		outputs[(size_t)MMC1_Output::CHR_A16] = BaseLogic::FromByte( ((!reg[0].b4 || !ppu_a12) && reg[1].b4) || ((reg[0].b4 && ppu_a12) && reg[2].b4));		// 22-aon

		// PRG Bank Switch

		outputs[(size_t)MMC1_Output::PRG_A14] = BaseLogic::FromByte( ((reg[3].b0 || !reg[0].b3 || reg[0].b2) && a14 && 1) || (reg[0].b2 && reg[0].b3 && reg[3].b0));		// 33-aon
		outputs[(size_t)MMC1_Output::PRG_A15] = BaseLogic::FromByte( ( ( (reg[0].b2 || !reg[0].b3) && reg[3].b1) && a14) || ( (reg[3].b1 && reg[0].b3) || reg[0].b2));		// aon
		outputs[(size_t)MMC1_Output::PRG_A16] = BaseLogic::FromByte( ( ( (reg[0].b2 || !reg[0].b3) && reg[3].b2) && a14) || ( (reg[3].b2 && reg[0].b3) || reg[0].b2));		// aon
		outputs[(size_t)MMC1_Output::PRG_A17] = BaseLogic::FromByte( ( ( (!reg[0].b3 && reg[3].b3) || (reg[0].b2 && reg[3].b3)) && !reg[3].b4) || ( ((!reg[3].b4 && reg[3].b3 && 1) || (!reg[3].b4 && reg[0].b2 && reg[0].b3)) && a14) || (reg[3].b3 && reg[3].b4));			// 222-aon

		// CIRAM A10 Line

		outputs[(size_t)MMC1_Output::CIRAM_A10] = BaseLogic::FromByte((ppu_a11 && reg[0].b0 && reg[0].b1) || (!reg[0].b0 && reg[0].b1 && ppu_a10) || (1 && reg[0].b0 && !reg[0].b1));		// 333-aon

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
}
