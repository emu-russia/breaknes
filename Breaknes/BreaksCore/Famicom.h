#pragma once

namespace Breaknes
{
	class Famicom
	{
		M6502Core::M6502* core = nullptr;
		BaseLogic::TriState CLK = BaseLogic::TriState::Zero;

		uint16_t addr_bus = 0;
		uint8_t data_bus = 0;

	public:
		Famicom();
		~Famicom();

		void sim();
	};
}
