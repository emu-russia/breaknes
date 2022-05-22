// LS139
// Dual 2-to-4 line decoder/demultiplexer

// https://github.com/emu-russia/breaks/blob/master/Docs/Famicom/LR74HC139.pdf

#include "pch.h"

using namespace BaseLogic;

namespace BaseBoard
{
	void LS139::sim(TriState inputs[], TriState n_Y1[4], TriState n_Y2[4])
	{
		TriState EN1 = NOT(inputs[(size_t)LS139_Input::n_EN1]);
		TriState EN2 = NOT(inputs[(size_t)LS139_Input::n_EN2]);

		TriState A1_0 = inputs[(size_t)LS139_Input::A1_0];
		TriState A1_1 = inputs[(size_t)LS139_Input::A1_1];
		TriState A2_0 = inputs[(size_t)LS139_Input::A2_0];
		TriState A2_1 = inputs[(size_t)LS139_Input::A2_1];

		n_Y1[0] = NAND3(NOT(A1_0), NOT(A1_1), EN1);
		n_Y1[1] = NAND3(A1_0, NOT(A1_1), EN1);
		n_Y1[2] = NAND3(NOT(A1_0), A1_1, EN1);
		n_Y1[3] = NAND3(A1_0, A1_1, EN1);

		n_Y2[0] = NAND3(NOT(A2_0), NOT(A2_1), EN2);
		n_Y2[1] = NAND3(A2_0, NOT(A2_1), EN2);
		n_Y2[2] = NAND3(NOT(A2_0), A2_1, EN2);
		n_Y2[3] = NAND3(A2_0, A2_1, EN2);
	}
}
