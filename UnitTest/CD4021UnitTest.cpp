#include "pch.h"

using namespace BaseLogic;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	CD4021_Test::CD4021_Test()
	{
	}

	CD4021_Test::~CD4021_Test()
	{
	}

	bool CD4021_Test::TestSequence(uint8_t test_vector)
	{
		TriState Q5, Q6, Q7;
		TriState clk = TriState::Zero;
		TriState par_ser{};
		uint8_t q = 0;
		char text[0x100]{};

		sprintf_s(text, sizeof(text), "Testing CD4021 with vector: 0x%02x\n", test_vector);
		Logger::WriteMessage(text);

		// Latch test vector
		par_ser = TriState::One;
		shift_reg.sim(clk, par_ser, TriState::Zero, test_vector, Q5, Q6, Q7);

		// Test serial output
		par_ser = TriState::Zero;

		for (int i = 0; i < 8; i++) {

			clk = TriState::Zero;
			shift_reg.sim(clk, par_ser, TriState::Zero, test_vector, Q5, Q6, Q7);

			sprintf_s(text, sizeof(text), "Q7=%d on clk %d\n", ToByte(Q7), i);
			Logger::WriteMessage(text);

			q |= ToByte(Q7) << (7 - i);

			// posedge -> Shift reg
			clk = TriState::One;
			shift_reg.sim(clk, par_ser, TriState::Zero, test_vector, Q5, Q6, Q7);
		}

		sprintf_s(text, sizeof(text), "Pushed out vector: 0x%02x\n", q);
		Logger::WriteMessage(text);

		return test_vector == q;
	}
}
