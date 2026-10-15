#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	void AddressBus::sim_ConstGen()
	{
		bool Z_ADL0 = core->cmd.Z_ADL0;
		bool Z_ADL1 = core->cmd.Z_ADL1;
		bool Z_ADL2 = core->cmd.Z_ADL2;
		bool Z_ADH0 = core->cmd.Z_ADH0;
		bool Z_ADH17 = core->cmd.Z_ADH17;

		if (Z_ADL0)
		{
			core->ADL &= ~0b00000001;
		}

		if (Z_ADL1)
		{
			core->ADL &= ~0b00000010;
		}

		if (Z_ADL2)
		{
			core->ADL &= ~0b00000100;
		}

		if (Z_ADH0)
		{
			core->ADH &= ~0b00000001;
		}

		if (Z_ADH17)
		{
			core->ADH &= ~0b11111110;
		}
	}

	void AddressBus::sim_Output(uint16_t* addr_bus)
	{
		sim_OutputImpl<0>(addr_bus);
	}

	template<int Phase> void AddressBus::sim_OutputImpl(uint16_t* addr_bus)
	{
		using P = Phi<Phase>;
		TriState PHI1 = core->wire.PHI1;
		bool ADL_ABL = core->cmd.ADL_ABL;
		bool ADH_ABH = core->cmd.ADH_ABH;

		// The address bus is set during PHI1 only

		if (P::Phi1One(PHI1) && ADL_ABL)
		{
			ABL = core->ADL;
		}

		if (P::Phi1One(PHI1) && ADH_ABH)
		{
			ABH = core->ADH;
		}

		uint16_t addr = ((uint16_t)ABH << 8) | ABL;
		*addr_bus = addr;
	}
	template void AddressBus::sim_OutputImpl<0>(uint16_t*);
	template void AddressBus::sim_OutputImpl<1>(uint16_t*);
	template void AddressBus::sim_OutputImpl<2>(uint16_t*);

	uint8_t AddressBus::getABL()
	{
		return ABL;
	}

	uint8_t AddressBus::getABH()
	{
		return ABH;
	}

	void AddressBus::setABL(uint8_t val)
	{
		ABL = val;
	}

	void AddressBus::setABH(uint8_t val)
	{
		ABH = val;
	}
}
