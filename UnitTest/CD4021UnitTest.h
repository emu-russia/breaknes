#pragma once

namespace UnitTest
{
	class CD4021_Test
	{
		IO::CD4021 shift_reg{};

	public:
		CD4021_Test();
		~CD4021_Test();

		bool TestSequence(uint8_t test_vector);
	};
}
