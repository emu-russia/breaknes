// Unit tests for the MMC1 chip (moved from Mappers to Chips, issue #509).
//
// The register protocol and the output formulas were cross-checked against the
// restored die netlist (emu-russia/mappers MMC1/deroute): 5 writes per register
// (bit 0 first), the transfer strobe fires when the divider count reaches 5,
// the divider is held at 0 by the D7 reset write and after each transfer, and
// the control register is written normally (only its bits 2,3 are forced by the
// D7 reset).

#include "pch.h"

#include "../Chips/MMC1/MMC1.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BaseLogic;

namespace UnitTest
{
	namespace
	{
		using namespace Chips;

		void SetInputs(TriState inputs[], bool m2, bool a13, bool a14, bool d0, bool d7, bool rnw, bool n_romsel)
		{
			for (size_t n = 0; n < (size_t)MMC1_Input::Max; n++)
			{
				inputs[n] = TriState::Zero;
			}
			inputs[(size_t)MMC1_Input::M2] = m2 ? TriState::One : TriState::Zero;
			inputs[(size_t)MMC1_Input::CPU_A13] = a13 ? TriState::One : TriState::Zero;
			inputs[(size_t)MMC1_Input::CPU_A14] = a14 ? TriState::One : TriState::Zero;
			inputs[(size_t)MMC1_Input::CPU_D0] = d0 ? TriState::One : TriState::Zero;
			inputs[(size_t)MMC1_Input::CPU_D7] = d7 ? TriState::One : TriState::Zero;
			inputs[(size_t)MMC1_Input::CPU_RnW] = rnw ? TriState::One : TriState::Zero;
			inputs[(size_t)MMC1_Input::nROMSEL] = n_romsel ? TriState::One : TriState::Zero;
			// PPU_A10/A11/A12 default to 0; tests override them when needed.
		}

		// Run one idle M2 cycle (strobe deasserted) to let the write latch clear.
		void IdleCycle(MMC1& mmc, TriState inputs[], TriState outputs[])
		{
			SetInputs(inputs, true, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
			SetInputs(inputs, false, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
		}

		// Release the initial divider hold, then perform a D7 = 1 reset write.
		void ResetMMC1(MMC1& mmc, TriState inputs[], TriState outputs[])
		{
			IdleCycle(mmc, inputs, outputs);
			// D7 = 1 write: asynchronously resets the divider and forces control bits 2,3.
			SetInputs(inputs, true, false, false, false, true, false, false);
			mmc.sim(inputs, outputs);
			SetInputs(inputs, false, false, false, false, true, false, false);
			mmc.sim(inputs, outputs);
			IdleCycle(mmc, inputs, outputs);
		}

		// One 5-bit MMC1 register write: bits are written LSB first (bit 0..bit 4),
		// matching the real chip. The register is selected with A13/A14.
		void WriteReg(MMC1& mmc, TriState inputs[], TriState outputs[], bool a13, bool a14, uint8_t value)
		{
			for (int bit = 0; bit < 5; bit++)
			{
				bool d0 = ((value >> bit) & 1) != 0;
				// M2 high with the write strobe asserted, then M2 low: the falling
				// edge of M2 clocks the shift register and the divider.
				SetInputs(inputs, true, a13, a14, d0, false, false, false);
				mmc.sim(inputs, outputs);
				SetInputs(inputs, false, a13, a14, d0, false, false, false);
				mmc.sim(inputs, outputs);
				// Deassert the strobe and let the write latch clear before the next bit.
				IdleCycle(mmc, inputs, outputs);
			}
		}
	}

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

			// A fresh chip has all registers at 0: the CHR bank outputs are all low.
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
			// A fresh MMC1 powers up with the control register at $0C (PRG mode 3):
			// the $8000 window is bank 0 (reg3 = 0), the $C000 window is the fixed
			// last bank (all ones, masked by the ROM size).
			Chips::MMC1 mmc;

			Assert::IsTrue(mmc.Dbg_GetPRGAddress(0x8000) == 0x0000);
			Assert::IsTrue(mmc.Dbg_GetPRGAddress(0x8000 + 0x1234) == 0x1234);
			Assert::IsTrue(mmc.Dbg_GetPRGAddress(0xC000 + 0x1234) == 0x3C000 + 0x1234);
		}

		TEST_METHOD(TestControlRegisterWrite)
		{
			// A normal 5-bit write to the control register ($8000-$9FFF) must update
			// mirroring (bits 0-1), PRG mode (bits 2-3) and CHR mode (bit 4). The D7
			// reset write only forces bits 2,3 (PRG mode 3).
			Chips::MMC1 mmc;
			TriState inputs[(size_t)Chips::MMC1_Input::Max]{};
			TriState outputs[(size_t)Chips::MMC1_Output::Max]{};

			ResetMMC1(mmc, inputs, outputs);

			// $0E = vertical mirroring (10) + PRG mode 3 (11) + CHR 8K (0).
			WriteReg(mmc, inputs, outputs, false, false, 0x0E);

			// Vertical mirroring: VRAM_A10 follows PPU_A10.
			inputs[(size_t)Chips::MMC1_Input::PPU_A10] = TriState::One;
			inputs[(size_t)Chips::MMC1_Input::PPU_A11] = TriState::Zero;
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::VRAM_A10] == TriState::One);
			inputs[(size_t)Chips::MMC1_Input::PPU_A10] = TriState::Zero;
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::VRAM_A10] == TriState::Zero);

			// $0F = horizontal mirroring (11): VRAM_A10 follows PPU_A11.
			WriteReg(mmc, inputs, outputs, false, false, 0x0F);
			inputs[(size_t)Chips::MMC1_Input::PPU_A10] = TriState::Zero;
			inputs[(size_t)Chips::MMC1_Input::PPU_A11] = TriState::One;
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::VRAM_A10] == TriState::One);
			inputs[(size_t)Chips::MMC1_Input::PPU_A11] = TriState::Zero;
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::VRAM_A10] == TriState::Zero);

			// PRG mode 3 (fixed 16K at $C000, switchable at $8000): at $8000 the
			// upper PRG address equals the bank register bits: A14..A17 = reg3.b0..b3.
			WriteReg(mmc, inputs, outputs, true, true, 0x05);	// reg3 = 00101
			SetInputs(inputs, true, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A14] == TriState::One);	// reg3.b0
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A15] == TriState::Zero);	// reg3.b1
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A16] == TriState::One);	// reg3.b2
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::Zero);	// reg3.b3
		}

		TEST_METHOD(TestCHRBankSwitch)
		{
			// CHR bank outputs per the deroute netlist (b4 = CHR mode bit):
			//   CHR_A12 = (A12 & (reg2.b0 | ~b4)) | (~A12 & b4 & reg1.b0)
			//   CHR_A13..A16 = ((~b4 | ~A12) & reg1.i) | (b4 & A12 & reg2.i)
			Chips::MMC1 mmc;
			TriState inputs[(size_t)Chips::MMC1_Input::Max]{};
			TriState outputs[(size_t)Chips::MMC1_Output::Max]{};

			ResetMMC1(mmc, inputs, outputs);

			// 8K CHR mode (b4 = 0): the single 8K bank comes from reg1 for all A12.
			WriteReg(mmc, inputs, outputs, false, false, 0x0E);
			WriteReg(mmc, inputs, outputs, true, false, 0x1A);	// reg1 = 11010
			WriteReg(mmc, inputs, outputs, false, true, 0x03);	// reg2 = 00011 (unused in 8K mode)

			inputs[(size_t)Chips::MMC1_Input::PPU_A12] = TriState::Zero;
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A12] == TriState::Zero);	// = PPU_A12
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A13] == TriState::One);	// reg1.b1
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A14] == TriState::Zero);	// reg1.b2
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A15] == TriState::One);	// reg1.b3
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A16] == TriState::One);	// reg1.b4

			inputs[(size_t)Chips::MMC1_Input::PPU_A12] = TriState::One;
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A12] == TriState::One);	// = PPU_A12
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A13] == TriState::One);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A14] == TriState::Zero);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A15] == TriState::One);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A16] == TriState::One);

			// 4K x 2 CHR mode (b4 = 1): PPU_A12 selects reg1 (low 4K) / reg2 (high 4K).
			WriteReg(mmc, inputs, outputs, false, false, 0x1E);
			WriteReg(mmc, inputs, outputs, true, false, 0x05);	// reg1 = 00101
			WriteReg(mmc, inputs, outputs, false, true, 0x0A);	// reg2 = 01010

			inputs[(size_t)Chips::MMC1_Input::PPU_A12] = TriState::Zero;
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A12] == TriState::One);	// reg1.b0
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A13] == TriState::Zero);	// reg1.b1
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A14] == TriState::One);	// reg1.b2
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A15] == TriState::Zero);	// reg1.b3
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A16] == TriState::Zero);	// reg1.b4

			inputs[(size_t)Chips::MMC1_Input::PPU_A12] = TriState::One;
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A12] == TriState::Zero);	// reg2.b0
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A13] == TriState::One);	// reg2.b1
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A14] == TriState::Zero);	// reg2.b2
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A15] == TriState::One);	// reg2.b3
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A16] == TriState::Zero);	// reg2.b4
		}

		TEST_METHOD(TestDividerResetsBetweenRegisters)
		{
			// After a transfer the divider is held at 0, so a second register write
			// sequence must start counting from 1 again (it must not continue at
			// 6,7,0,... which would silently drop the second register).
			Chips::MMC1 mmc;
			TriState inputs[(size_t)Chips::MMC1_Input::Max]{};
			TriState outputs[(size_t)Chips::MMC1_Output::Max]{};

			ResetMMC1(mmc, inputs, outputs);

			// PRG register first (reg3 = 1).
			WriteReg(mmc, inputs, outputs, true, true, 0x01);

			// PRG mode 3 (from the reset), $8000: PRG_A14 = reg3.b0 = 1, PRG_A17 = reg3.b3 = 0.
			SetInputs(inputs, true, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A14] == TriState::One);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::Zero);

			// A second register (CHR bank 0) must transfer as well.
			WriteReg(mmc, inputs, outputs, true, false, 0x0F);	// reg1 = 01111

			// 8K CHR mode (b4 = 0 after the reset): CHR_A13..A16 = reg1.b1..b4.
			SetInputs(inputs, true, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A13] == TriState::One);	// reg1.b1
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A14] == TriState::One);	// reg1.b2
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A15] == TriState::One);	// reg1.b3
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::CHR_A16] == TriState::Zero);	// reg1.b4
		}

		TEST_METHOD(TestPRGModeTable)
		{
			// Standard MMC1 PRG modes (reg3 = 5 = 00101 in all cases; the control
			// register bits: b2/b3 = PRG mode, so mode 2 = $04, mode 3 = $0C):
			//   mode 0/1 (b2=0): 32K window at (reg3 & 0x0E) << 14, bit 0 ignored
			//   mode 2 ($04):    $8000 = bank 0 fixed, $C000 = reg3 & 0x0F
			//   mode 3 ($0C):    $8000 = reg3 & 0x0F, $C000 = last bank
			Chips::MMC1 mmc;
			TriState inputs[(size_t)Chips::MMC1_Input::Max]{};
			TriState outputs[(size_t)Chips::MMC1_Output::Max]{};

			IdleCycle(mmc, inputs, outputs);	// release the initial divider hold
			WriteReg(mmc, inputs, outputs, false, false, 0x00);	// mode 0
			WriteReg(mmc, inputs, outputs, true, true, 0x05);	// reg3 = 5

			// Mode 0: 32K window at (5 & 0x0E) << 14 = 4 << 14.
			// $8000 (a14 = 0): A14..A17 = 0, reg3.b1, reg3.b2, reg3.b3 = 0100.
			SetInputs(inputs, true, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A14] == TriState::Zero);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A15] == TriState::Zero);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A16] == TriState::One);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::Zero);
			// $C000 (a14 = 1): A14 = 1 -> 0101 (upper half of the 32K window).
			SetInputs(inputs, true, false, true, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A14] == TriState::One);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A15] == TriState::Zero);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A16] == TriState::One);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::Zero);

			// Mode 2 ($04): $8000 fixed at bank 0, $C000 = reg3.
			WriteReg(mmc, inputs, outputs, false, false, 0x04);
			SetInputs(inputs, true, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A14] == TriState::Zero);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A15] == TriState::Zero);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A16] == TriState::Zero);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::Zero);
			SetInputs(inputs, true, false, true, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A14] == TriState::One);	// reg3.b0
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A15] == TriState::Zero);	// reg3.b1
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A16] == TriState::One);	// reg3.b2
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::Zero);	// reg3.b3

			// Mode 3 ($0C): $8000 = reg3, $C000 = last bank (all ones).
			WriteReg(mmc, inputs, outputs, false, false, 0x0C);
			SetInputs(inputs, true, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A14] == TriState::One);	// reg3.b0
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A15] == TriState::Zero);	// reg3.b1
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A16] == TriState::One);	// reg3.b2
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::Zero);	// reg3.b3
			SetInputs(inputs, true, false, true, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A14] == TriState::One);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A15] == TriState::One);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A16] == TriState::One);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::One);
		}

		TEST_METHOD(TestD7ResetHoldsDivider)
		{
			// A D7 write resets the divider without advancing it: after it, a full
			// 5-bit write must transfer the exact value (with the D7 write wrongly
			// consuming a count, the transfer would capture a shifted value).
			Chips::MMC1 mmc;
			TriState inputs[(size_t)Chips::MMC1_Input::Max]{};
			TriState outputs[(size_t)Chips::MMC1_Output::Max]{};

			IdleCycle(mmc, inputs, outputs);	// release the initial divider hold
			WriteReg(mmc, inputs, outputs, true, true, 0x1D);	// reg3 = 11101

			// reg3.b3 = 1: at $8000, PRG_A17 = reg3.b3 = 1.
			SetInputs(inputs, true, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::One);

			// D7 reset, then write reg3 = 2: the counter must restart from 0.
			ResetMMC1(mmc, inputs, outputs);
			WriteReg(mmc, inputs, outputs, true, true, 0x02);	// reg3 = 00010

			SetInputs(inputs, true, false, false, false, false, true, true);
			mmc.sim(inputs, outputs);
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A14] == TriState::Zero);	// reg3.b0 = 0
			Assert::IsTrue(outputs[(size_t)Chips::MMC1_Output::PRG_A17] == TriState::Zero);	// reg3.b3 = 0
		}
	};
}
