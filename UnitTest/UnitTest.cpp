#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(UnitTest)
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
}
