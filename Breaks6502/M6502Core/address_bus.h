#pragma once

namespace M6502Core
{
	class AddressBus
	{
		uint8_t ABL = 0;
		uint8_t ABH = 0;

		M6502* core = nullptr;

	public:

		AddressBus(M6502* parent) { core = parent; }

		void sim_ConstGen();

		void sim_Output(uint16_t *addr_bus);

		uint8_t getABL();
		uint8_t getABH();
	};
}
