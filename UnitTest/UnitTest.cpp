#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BaseLogic;

namespace UnitTest
{
	TEST_CLASS(BaseLogicUnitTest)
	{
	public:

		TEST_METHOD(TestBitRev)
		{
			TriState bits[8]{};
			Unpack(0b10000110, bits);
			BitRev(bits);
			Assert::IsTrue(Pack(bits) == 0b01100001);
		}

		TEST_METHOD(TestMUX2)
		{
			TriState in[4]{};
			TriState sel[2]{};

			for (size_t n = 0; n < _countof(in); n++)
			{
				sel[0] = n & 1 ? TriState::One : TriState::Zero;
				sel[1] = n & 2 ? TriState::One : TriState::Zero;

				for (size_t i = 0; i < _countof(in); i++)
				{
					in[i] = TriState::Zero;
				}

				in[n] = TriState::One;

				Assert::IsTrue(MUX2(sel, in) == TriState::One);
			}
		}

		TEST_METHOD(TestMUX3)
		{
			TriState in[8]{};
			TriState sel[3]{};

			for (size_t n = 0; n < _countof(in); n++)
			{
				sel[0] = n & 1 ? TriState::One : TriState::Zero;
				sel[1] = n & 2 ? TriState::One : TriState::Zero;
				sel[2] = n & 4 ? TriState::One : TriState::Zero;

				for (size_t i = 0; i < _countof(in); i++)
				{
					in[i] = TriState::Zero;
				}

				in[n] = TriState::One;

				Assert::IsTrue(MUX3(sel, in) == TriState::One);
			}
		}
	};

	TEST_CLASS(CoreUnitTest)
	{
	public:
		
		TEST_METHOD(TestProgramCounter)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.PC_UnitTest());
		}

		TEST_METHOD(TestALU)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.ALU_UnitTest());
		}
	};

	TEST_CLASS(ApuUnitTest)
	{
	public:

		TEST_METHOD(TestCounters)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03);
			Assert::IsTrue(ut.TestCounters());
		}

		TEST_METHOD(TestDiv)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03);
			Assert::IsTrue(ut.TestDiv(false));
		}

		TEST_METHOD(TestAclk)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03);
			Assert::IsTrue(ut.TestAclk());
		}

		TEST_METHOD(TestLFO)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03);
			Assert::IsTrue(ut.TestLFO(false));
			Assert::IsTrue(ut.TestLFO(true));
		}

		TEST_METHOD(TestRegOps)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03);
			Assert::IsTrue(ut.TestRegOps());
		}

		TEST_METHOD(TestLengthDecoder)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03);
			Assert::IsTrue(ut.TestLengthDecoder());
		}

		TEST_METHOD(TestLengthCounter)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03);
			Assert::IsTrue(ut.TestLengthCounter());
		}
	};

	TEST_CLASS(PpuUnitTest)
	{
	public:

		TEST_METHOD(TestHVCounter_Reset)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.HVCounterTestRES());
		}

		TEST_METHOD(TestHVCounter_Count)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.HVCounterTest());
		}

		TEST_METHOD(One_PCLK)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.RunSinglePCLK());
		}

		TEST_METHOD(One_ScanLine)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.RunLines(1));
		}

		TEST_METHOD(RunHalfCycles_WithSignals)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.RunHalfCyclesWithChipScope(10000, "RP2C02G_line.json"));
		}

		TEST_METHOD(One_Field)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.RunLines(262));
		}

		/// <summary>
		/// A test to find out why rows 2 and 6 are not used for OAM Buffer 2-4 bits.
		/// It turned out that these rows fall on the attribute byte of the sprite in which bits 2-4 are not used.
		/// </summary>
		TEST_METHOD(Dump_OAMAddress_ROW)
		{
			for (size_t OamAddr = 0; OamAddr < (64 * 4); OamAddr++)
			{
				size_t row = OamAddr & 7;
				Logger::WriteMessage(("OamAddr: " + std::to_string(OamAddr) + ", row: " + std::to_string(row)).c_str());
				if ((OamAddr % 4) == 2)
				{
					Logger::WriteMessage(" ATTR ");
				}
				if (row == 2 || row == 6)
				{
					Logger::WriteMessage(" UNUSED for OB[2-4]");
				}
				Logger::WriteMessage("\n");
			}
		}

		TEST_METHOD(TestComparator)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);

			for (size_t a=0; a<0x100; a++)
			{
				for (size_t b = 0; b < 0x100; b++)
				{
					uint8_t res = ut.TestComparator((uint8_t)a, (uint8_t)b);
					uint8_t should = (uint8_t)((int8_t)(uint8_t)a - (int8_t)(uint8_t)b);
					if (res != should)
					{
						Logger::WriteMessage((std::to_string(a) + " - " + std::to_string(b) + " = "
							+ std::to_string(res) + ", should: " + std::to_string(should)).c_str());
					}
					Assert::IsTrue(res == should);
				}
			}
		}

		TEST_METHOD(TestBGC_SR8)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestBGC_SR8());
		}

		TEST_METHOD(TestOAMCounter)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestOAMCounter());
		}

		TEST_METHOD(TestFIFODownCounter)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			for (size_t n = 0; n <= 0xff; n++)
			{
				Logger::WriteMessage(("Value: " + std::to_string(n) + "\n").c_str());
				Assert::IsTrue(ut.TestFIFODownCounter(n));
			}
		}

		TEST_METHOD(TestFIFOPairedSR)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			for (size_t n = 0; n <= 0xff; n++)
			{
				Logger::WriteMessage(("Value: " + std::to_string(n) + "\n").c_str());
				Assert::IsTrue(ut.TestFIFOPairedSR((uint8_t)n));
			}
		}

		TEST_METHOD(TestNtscPpuChromaDecoderOutputs)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestNtscPpuChromaDecoderOutputs());
		}

		TEST_METHOD(DumpNtscPpuPhaseShifter)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			ut.DumpNtscPpuPhaseShifter(false);
		}

		TEST_METHOD(DumpNtscPpuColorSpace)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			ut.DumpNtscPpuColorSpace(false);
		}

		TEST_METHOD(Dump_2C04_0003_PpuColorSpace)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C04_0003);
			ut.Dump_2C04_0003_ColorSpace();
		}
	};
}
