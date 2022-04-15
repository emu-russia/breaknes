#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
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

	};
}
