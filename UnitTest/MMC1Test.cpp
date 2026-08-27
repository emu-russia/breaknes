// Unit tests for the MMC1 chip (moved from Mappers to Chips, issue #509).

#include "pch.h"

#include "../Chips/MMC1/MMC1.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BaseLogic;

namespace UnitTest
{
	TEST_CLASS(MMC1UnitTest)
	{
	public:
		TEST_METHOD(TestPinNamesMatchEnums)
		{
			// The pin name tables must stay in sync with the enums (the CartPcb
			// netlist addresses the chip pins by name).
			Assert::IsTrue(sizeof(Chips::MMC1_InputPinNames) / sizeof(Chips::MMC1_InputPinNames[0]) == (size_t)Chips::MMC1_Input::Max);
			Assert::IsTrue(sizeof(Chips::MMC1_OutputPinNames) / sizeof(Chips::MMC1_OutputPinNames[0]) == (size_t)Chips::MMC1_Output::Max);

			Assert::IsTrue(std::string(Chips::MMC1_OutputPinNames[(size_t)Chips::MMC1_Output::VRAM_A10]) == "VRAM_A10");
			Assert::IsTrue(std::string(Chips::MMC1_OutputPinNames[(size_t)Chips::MMC1_Output::PRG_nCE]) == "PRG_nCE");
		}

		TEST_METHOD(TestResetState)
		{
			Chips::MMC1 mmc;

			TriState inputs[(size_t)Chips::MMC1_Input::Max]{};
			TriState outputs[(size_t)Chips::MMC1_Output::Max]{};

			// Idle bus: no access (M2 high, /ROMSEL high, RnW high, low address bits).
			for (size_t n = 0; n < (size_t)Chips::MMC1_Input::Max; n++)
			{
				inputs[n] = TriState::One;
			}
			inputs[(size_t)Chips::MMC1_Input::CPU_A13] = TriState::Zero;
			inputs[(size_t)Chips::MMC1_Input::CPU_A14] = TriState::Zero;
			inputs[(size_t)Chips::MMC1_Input::PPU_A10] = TriState::Zero;
			inputs[(size_t)Chips::MMC1_Input::PPU_A11] = TriState::Zero;
			inputs[(size_t)Chips::MMC1_Input::PPU_A12] = TriState::Zero;

			mmc.sim(inputs, outputs);

			// A fresh chip has all registers at 0 (the mode bits reg0.b2/b3 are
			// forced to 1 by the internal reset, i.e. PRG mode 3, but the bank
			// registers 1-3 are 0): the CHR bank outputs are all low.
			for (size_t n = (size_t)Chips::MMC1_Output::CHR_A12; n <= (size_t)Chips::MMC1_Output::CHR_A16; n++)
			{
				Assert::IsTrue(outputs[n] == TriState::Zero);
			}

			// SRAM is not selected on an idle bus (A13/A14 low); PRG is not
			// selected while /ROMSEL is high.
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::SRAM_CE] == TriState::Zero);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_nCE] == TriState::One);
		}

		TEST_METHOD(TestDefaultPRGAddress)
		{
			// A fresh MMC1 has all registers at 0: PRG is unswitched (bank 0).
			Chips::MMC1 mmc;

			Assert::IsTrue(mmc.Dbg_GetPRGAddress(0x8000) == 0x0000);
			Assert::IsTrue(mmc.Dbg_GetPRGAddress(0x8000 + 0x1234) == 0x1234);
			Assert::IsTrue(mmc.Dbg_GetPRGAddress(0xC000 + 0x1234) == 0x4000 + 0x1234);
		}
	};
}
