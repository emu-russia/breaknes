#pragma once

namespace M6502CoreUnitTest
{
	class UnitTest
	{
		enum class ALU_Operation
		{
			None = 0,
			ANDS,
			EORS,
			ORS,
			SRS,
			SUMS,
		};

		void ResetPcInputs();
		bool PC_Test(uint16_t initial_pc, uint16_t expected_pc, bool inc, const char* test_name);

		void ResetALUInputs(ALU_Operation op);
		int TestCompute(uint8_t a, uint8_t b, uint8_t expected, ALU_Operation op, bool bcd, bool carry);
		size_t BCD_Add(size_t a, size_t b, bool carry_in);

		M6502Core::M6502 *core;

	public:
		UnitTest();
		~UnitTest();

		/// <summary>
		/// Checking Program Counter simulator functionality.
		/// </summary>
		bool PC_UnitTest();

		/// <summary>
		/// Checking the operability of the ALU 6502 in different modes.
		/// </summary>
		/// <returns></returns>
		bool ALU_UnitTest();

		/// <summary>
		/// Go through all 6502 decoder values and output them to CSV.
		/// </summary>
		/// <returns></returns>
		bool DumpDecoder();
	};
}
