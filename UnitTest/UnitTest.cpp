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
	};
}
