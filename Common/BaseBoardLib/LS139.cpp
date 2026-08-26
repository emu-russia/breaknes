// LS139
// Dual 2-to-4 line decoder/demultiplexer

// https://github.com/emu-russia/breaks/blob/master/Docs/Famicom/LR74HC139.pdf

#include "pch.h"

using namespace BaseLogic;

namespace BaseBoard
{
	void LS139::sim(
		TriState& n_EN1,
		TriState& n_EN2,
		TriState& A1_0,
		TriState& A1_1,
		TriState& A2_0,
		TriState& A2_1,
		TriState n_Y1[4],
		TriState n_Y2[4])
	{
		// 74LS139 inputs have internal pull-up resistors, so a floating (Z) input reads as a logic 1.
		// This matters e.g. for the M2 line, which the CPU drives to Z during reset; the pull-up is
		// inside this chip, so the caller does not have to pull the net up externally anymore.

		Pullup(n_EN1);
		Pullup(n_EN2);
		Pullup(A1_0);
		Pullup(A1_1);
		Pullup(A2_0);
		Pullup(A2_1);

		TriState EN1 = NOT(n_EN1);
		n_Y1[0] = NAND3(NOT(A1_0), NOT(A1_1), EN1);
		n_Y1[1] = NAND3(A1_0, NOT(A1_1), EN1);
		n_Y1[2] = NAND3(NOT(A1_0), A1_1, EN1);
		n_Y1[3] = NAND3(A1_0, A1_1, EN1);

		TriState EN2 = NOT(n_EN2);
		n_Y2[0] = NAND3(NOT(A2_0), NOT(A2_1), EN2);
		n_Y2[1] = NAND3(A2_0, NOT(A2_1), EN2);
		n_Y2[2] = NAND3(NOT(A2_0), A2_1, EN2);
		n_Y2[3] = NAND3(A2_0, A2_1, EN2);
	}
}
