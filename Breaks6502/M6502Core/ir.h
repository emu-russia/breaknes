#pragma once

namespace M6502Core
{
	class IR
	{
		uint8_t ir_latch = 0;

		M6502* core = nullptr;

	public:

		IR(M6502* parent) { core = parent; }

		uint8_t IROut = 0;

		void sim();
	};
}
