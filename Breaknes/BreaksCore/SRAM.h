// Generic SRAM

#pragma once

namespace Breaknes
{
	class SRAM
	{
		uint8_t* mem = nullptr;

	public:
		SRAM(size_t bits);
		~SRAM();

		void sim(BaseLogic::TriState n_CS, BaseLogic::TriState n_WE, BaseLogic::TriState n_OE, uint32_t *addr, uint8_t *data, bool& dz);
	};
}
