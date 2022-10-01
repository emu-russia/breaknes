// Debugging mechanisms of the APU.

#pragma once

namespace APUSim
{
	struct APU_Interconnects
	{
		uint8_t CLK;
	};

	/// <summary>
	/// Various registers, buffers and counters inside the APU.
	/// </summary>
	struct APU_Registers
	{
		uint32_t Bogus;
	};
}
